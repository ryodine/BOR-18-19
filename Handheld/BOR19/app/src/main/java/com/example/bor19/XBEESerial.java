package com.example.bor19;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Handler;

import com.felhr.usbserial.UsbSerialDevice;
import com.felhr.usbserial.UsbSerialInterface;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.PriorityQueue;
import java.util.Queue;

/**
 * Created by ryanjohnson on 3/26/18.
 */

public class XBEESerial {
    MainActivity context;
    UsbDevice device;
    String deviceKey;
    UsbDeviceConnection conn;
    UsbManager manager;
    UsbSerialDevice serial;
    BroadcastReceiver usbReceiver;
    public MessageBroker messenger;

    long timeSinceLastPing = 0;
    long timeSinceLastPong = 0;

    private static final int DISCONNECTED_PING_INTERVAL = 2000;
    private static final int ALLOWABLE_SILENCE_INTERVAL = 15000;

    public enum CommunicationState {
        DISCONNECTED,
        DEVICE_FOUND,
        PERMISSION_REQUEST,
        PERMISSION_DENIED,
        CONNECTION_INIT,
        WAIT_FOR_CONFIRMATION,
        CONNECTED,
    }

    public enum IncomingMessageStatus {
        GARBLED(400), ERROR(500), BUSY(503), OK(200);
        private final int id;
        IncomingMessageStatus(int id) { this.id = id; }
        public int getValue() { return id; }
    }

    public CommunicationState state = CommunicationState.DISCONNECTED;

    final String USB_PERMISSION_ACTION = "com.example.bor19.usbpermission";


    public XBEESerial(MainActivity context) {
        this.context = context;
        manager = (UsbManager) context.getSystemService(context.USB_SERVICE);
        messenger = new MessageBroker(this);

        timeSinceLastPong = System.currentTimeMillis();

        final MainActivity ctxt = context;
        usbReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (intent.getAction().equals(USB_PERMISSION_ACTION)) {
                    startConnection();
                }
            }
        };
    }

    private boolean startConnection() {
        if (manager.hasPermission(device)) {
            conn = manager.openDevice(device);
            serial = UsbSerialDevice.createUsbSerialDevice(device, conn);
            state = CommunicationState.CONNECTION_INIT;
            return true;
        }
        return false;
    }

    public void tick() {

        //Check for a disconnect
        if (state != CommunicationState.DISCONNECTED) {
            if (manager.getDeviceList().get(deviceKey) == null) {
                state = CommunicationState.DISCONNECTED;
                serial.close();
            }
        }

        if (state == CommunicationState.DISCONNECTED) {
            attemptConnection();
        } else if (state == CommunicationState.DEVICE_FOUND) {
            log("Device found");
            if (!startConnection()) {
                PendingIntent reqIntent = PendingIntent.getBroadcast(context, 0, new Intent(USB_PERMISSION_ACTION), 0);
                manager.requestPermission(device, reqIntent);

                IntentFilter filter = new IntentFilter(USB_PERMISSION_ACTION);
                context.registerReceiver(usbReceiver, filter);
                state = CommunicationState.PERMISSION_REQUEST;
            }
        } else if (state == CommunicationState.CONNECTION_INIT) {
            serial.open();
            serial.setBaudRate(9600);
            serial.read(new UsbSerialInterface.UsbReadCallback() {
                @Override
                public void onReceivedData(byte[] bytes) {
                    timeSinceLastPong = System.currentTimeMillis();
                    if (messenger.onReceive(bytes)) {
                        //
                    }
                }
            });
            state = CommunicationState.WAIT_FOR_CONFIRMATION;

            log("Waiting for handshake. Try resetting the arduino");
        } else if (state == CommunicationState.CONNECTED) {
            //handleAndroidCommunication(m);
            if (System.currentTimeMillis() - timeSinceLastPong > ALLOWABLE_SILENCE_INTERVAL) {
                state = CommunicationState.WAIT_FOR_CONFIRMATION;
            }
        } else if (state == CommunicationState.WAIT_FOR_CONFIRMATION) {
            if (messenger.messages.size() > 0) {
                MessageBroker.Message m = messenger.messages.poll();
                if (m.content_length > 0 && m.content_type.equals("text/plain") && new String(m.body).trim().equals("pong")) {
                    state = CommunicationState.CONNECTED;
                } else {
                    System.out.println("Expected PONG, got: ");
                    System.out.println(m);
                }
            }
            if (System.currentTimeMillis() - timeSinceLastPing > DISCONNECTED_PING_INTERVAL) {
                timeSinceLastPing = System.currentTimeMillis();
                messenger.sendPing();
            }
        }
    }

    public void stop() {
        if (serial != null) {
            serial.close();
            state = CommunicationState.DISCONNECTED;
        }
    }

    protected void send(byte[] msg) {
        if (state == CommunicationState.CONNECTED || state == CommunicationState.WAIT_FOR_CONFIRMATION) {
            serial.write(msg);
        }
    }

    private void attemptConnection() {

        log("Looking for device");
        HashMap<String, UsbDevice> deviceList = manager.getDeviceList();
        for (Map.Entry<String, UsbDevice> e : deviceList.entrySet()) {
            //log(e.getValue().getVendorId() + "");
            //log(e.getValue().getManufacturerName());
            if (e.getValue().getManufacturerName().equals("FTDI") && e.getValue().getVendorId() == 1027) {
                device = e.getValue();
                deviceKey = e.getKey();
                state = CommunicationState.DEVICE_FOUND;
            }
        }
    }

    private void log(final String me) {
        context.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                context.log(me);
            }
        });
    }

    // Message Broker Class
    class MessageBroker {

        private ByteArrayOutputStream buff = new ByteArrayOutputStream();
        private XBEESerial serial;
        public Queue<Message> messages = new LinkedList<>();

        public MessageBroker(XBEESerial s) {serial = s;}

        private int indexOf(byte[] outerArray, byte[] smallerArray) {
            for(int i = 0; i < outerArray.length - smallerArray.length+1; ++i) {
                boolean found = true;
                for(int j = 0; j < smallerArray.length; ++j) {
                    if (outerArray[i+j] != smallerArray[j]) {
                        found = false;
                        break;
                    }
                }
                if (found) return i;
            }
            return -1;
        }

        protected boolean onReceive(byte[] bytes) {
            try {
                buff.write(bytes);
            } catch (IOException e) {
                e.printStackTrace();
            }
            byte[] delimit = {'\r', '\n', '\r', '\n', '\r', '\n'};
            byte[] delimit2 = {'\r', '\n', '\r', '\n'};

            // check if message is done
            while (buff.toString().contains("\r\n\r\n\r\n")) {
                System.out.println(buff.toByteArray().length);
                System.out.println(buff.toString().length());
                byte[] arr = buff.toByteArray();
                try {
                    String buffer = buff.toString();

                    byte[] arr2 = new byte[arr.length];

                    int end_idx = indexOf(arr, delimit);
                    System.arraycopy(arr, 0, arr2, 0, arr.length);
                    buff.reset();
                    if (arr.length > end_idx + 6)
                        buff.write(arr, end_idx+6, arr.length-(end_idx+6));

                    byte[] msg_b = new byte[end_idx];
                    System.arraycopy(arr2, 0, msg_b, 0, end_idx);
                    Message m = new Message();

                    int hdr_end = indexOf(msg_b, delimit2);

                    byte hdr[] = new byte[hdr_end];
                    byte bod[] = new byte[msg_b.length-(hdr_end+delimit2.length)];
                    System.arraycopy(msg_b, 0, hdr, 0, hdr_end);
                    System.arraycopy(msg_b, hdr_end+delimit2.length, bod, 0, msg_b.length-(hdr_end+delimit2.length));



                    for (String line : new String(hdr).split("\n")) {
                        String[] split = line.split(": ");
                        switch (split[0].trim()) {
                            case "Content-Length":
                                m.content_length = Integer.parseInt(split[1].trim());
                                break;
                            case "Content-Type":
                                m.content_type = split[1].trim();
                            case "Status":
                                if (split[1].trim().equals("200 OK")) {
                                    m.status = IncomingMessageStatus.OK;
                                } else if (split[1].trim().equals("503 BUSY")) {
                                    m.status = IncomingMessageStatus.BUSY;
                                } else if (split[1].trim().equals("400 UNPROCESSABLE")) {
                                    m.status = IncomingMessageStatus.GARBLED;
                                } else if (split[1].trim().equals("500 ERROR")) {
                                    m.status = IncomingMessageStatus.ERROR;
                                }
                                break;
                        }
                    }
                    m.body = new byte[bod.length];
                    System.arraycopy(bod, 0, m.body, 0, bod.length);
                    //m.body = parts[1].getBytes();
                    m.raw = new String(msg_b);
                    messages.add(m);
                    return true;
                } catch (Exception e) {
                    System.out.println("Bad Message");
                    System.out.println(new String(arr));
                    e.printStackTrace();
                    return false;
                }
            }
            return false;
        }

        private void sendMessage(String action) {
            String message = "Action: " + action + "\r\n\r\n";
            serial.send(message.getBytes());
        }

        public void sendPing() {
            sendMessage("ping");
        }

        public void pollStatus() {
            sendMessage("status");
        }

        public void takePhoto() {
            sendMessage("photo");
        }

        class Message {
            public String content_type = "";
            public int content_length = 0;
            public byte[] body;
            public IncomingMessageStatus status = IncomingMessageStatus.ERROR;
            public String raw = "";

            @Override
            public String toString() {
                String ret = "";
                if (content_type == "") {
                    ret += "Content-Type: MISSING";
                } else {
                    ret += "Content-Type: " + content_type;
                }

                ret += "\nContent-Length: " + content_length;
                ret += "\nStatus: " + status;

                if (body == null) {
                    ret += "\nbody: MISSING";
                } else {
                    ret += "\nbody: " + new String(body);
                }

                return "\n" + ret;
            }
        }
    }

}
