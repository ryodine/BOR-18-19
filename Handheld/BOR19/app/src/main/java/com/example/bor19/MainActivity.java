package com.example.bor19;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    XBEESerial xbee;
    Looper mainloop;
    Thread loopThread;


    //UI

    View commStatus;
    View xbeeStatus;
    TextView status;
    TextView counter;
    ImageView photoview;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //UI
        commStatus = findViewById(R.id.comm_indicator);
        xbeeStatus = findViewById(R.id.xbee_indicator);
        status = findViewById(R.id.status_box);
        counter = findViewById(R.id.counter);
        photoview = findViewById(R.id.photo);

        findViewById(R.id.poll_status).setOnClickListener(v -> {
            if (xbee.state == XBEESerial.CommunicationState.CONNECTED) {
                xbee.messenger.pollStatus();
            }
        });

        findViewById(R.id.halt).setOnClickListener(v -> {
            xbee.stop();
            mainloop.shouldrun = false;
        });

        findViewById(R.id.photo_btn).setOnClickListener(v -> {
            if (xbee.state == XBEESerial.CommunicationState.CONNECTED) {
                xbee.messenger.takePhoto();
            }
        });

        findViewById(R.id.arm_btn).setOnClickListener(v -> {
            if (xbee.state == XBEESerial.CommunicationState.CONNECTED) {
                xbee.messenger.sendArm();
            }
        });

        findViewById(R.id.soil_btn).setOnClickListener(v -> {
            if (xbee.state == XBEESerial.CommunicationState.CONNECTED) {
                xbee.messenger.sendSoil();
            }
        });

        //Subsystems
        xbee = new XBEESerial(this);

        // Looping
        Handler h = new Handler();
        mainloop = new Looper(h);

        // Add iterative loops
        mainloop.addLoop(() -> xbee.tick());

        //Status indicator
        mainloop.addLoop(() -> {
            counter.setText("" + (System.currentTimeMillis() - xbee.timeSinceLastPong)/1000);
            switch (xbee.state) {
                case CONNECTED:
                    commStatus.setBackgroundColor(Color.GREEN);
                    xbeeStatus.setBackgroundColor(Color.GREEN);
                    break;
                case DEVICE_FOUND:
                    commStatus.setBackgroundColor(Color.RED);
                    xbeeStatus.setBackgroundColor(Color.YELLOW);
                    break;
                case CONNECTION_INIT:
                    commStatus.setBackgroundColor(Color.RED);
                    xbeeStatus.setBackgroundColor(Color.GREEN);
                    break;
                case WAIT_FOR_CONFIRMATION:
                    commStatus.setBackgroundColor(Color.YELLOW);
                    xbeeStatus.setBackgroundColor(Color.GREEN);
                    break;
                default:
                    commStatus.setBackgroundColor(Color.RED);
                    xbeeStatus.setBackgroundColor(Color.RED);
                    break;
            }
        });

        //Message listening
        mainloop.addLoop(() -> {
            if (xbee.messenger.messages.size() > 0) {
                XBEESerial.MessageBroker.Message m = xbee.messenger.messages.poll();
                if (m.content_type.equals("text/plain")) {
                    status.setText(new String(m.body));
                } else if (m.content_type.equals("image/jpeg")) {
                    Bitmap bm = BitmapFactory.decodeByteArray(m.body, 0, m.body.length);
                    DisplayMetrics dm = new DisplayMetrics();
                    getWindowManager().getDefaultDisplay().getMetrics(dm);

                    photoview.setMinimumHeight(dm.heightPixels);
                    photoview.setMinimumWidth(dm.widthPixels);
                    photoview.setImageBitmap(bm);
                }
            }
        });


        // Start all loops
        loopThread = new Thread(mainloop);
        loopThread.start();

        /*findViewById(R.id.connect_btn).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
               //
            }
        });*/
    }

    public void log(String m) {
        System.out.println(m);
    }

    private class Looper implements Runnable {
        public static final int UPDATE_INTERVAL = 200;

        private boolean shouldrun = true;
        private Handler h;

        private List<Runnable> loops = new ArrayList<Runnable>();

        public void addLoop(Runnable r) {
            loops.add(r);
        }

        public void abort() {
            shouldrun = false;
        }

        public Looper(Handler viewThreadHandler) {
            h = viewThreadHandler;
        }

        @Override
        public void run() {
            //mContext.log("Start time set");
            while (shouldrun) {
                h.post(new Runnable() {
                    @Override
                    public void run() {
                        for (Runnable r : loops) {
                            r.run();
                        }
                    }
                });
                try {
                    Thread.sleep(UPDATE_INTERVAL);
                } catch (InterruptedException e) {
                    //
                }
            }

            //Make it runnable again:
            shouldrun = true;

        }
    }
}
