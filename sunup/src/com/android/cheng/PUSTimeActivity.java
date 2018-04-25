package com.android.cheng; 
  
import java.util.Calendar;  
import android.app.Activity;  
import android.app.DatePickerDialog;  
import android.app.Dialog;  
import android.os.Bundle;  
import android.os.IBinder;
import android.os.Handler;  
import android.os.Message;  
import android.view.View;  
import android.widget.Button;  
import android.widget.DatePicker;  
import android.widget.TextView;  

import android.app.TimePickerDialog;
import android.widget.TimePicker;

import android.util.Log;
import android.content.Context;
import android.text.format.DateFormat;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.content.Intent;
import android.content.IntentFilter;  
import android.content.BroadcastReceiver;  
import android.content.DialogInterface;

import android.app.AlertDialog;
import android.app.AlertDialog.Builder;

public class PUSTimeActivity extends Activity {

    private Button uploadTime = null;
    private Button currentTime = null;
    private Button downloadTime = null;
    private TextView cpuTime = null;
    private TextView sysTime = null;

    private IComService mIComService;


    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage (Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case 1:
                    updateTime();
                    break;
                
                default:
                    break;
            }
        }
    };

    public class TimeThread extends Thread {
        @Override
        public void run () {
            do {
                try {
                    Thread.sleep(1000);
                    Message msg = new Message();
                    msg.what = 1;
                    mHandler.sendMessage(msg);
                }
                catch (InterruptedException e) {
                    e.printStackTrace();
                }
            } while(true);
        }
    }

    private ServiceConnection mServiceConnection = new ServiceConnection() {  
        public void onServiceConnected(ComponentName name, IBinder service) {  
            mIComService = IComService.Stub.asInterface(service);
        } 

        public void onServiceDisconnected(ComponentName name) {  
        }  
    };  

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver(){
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if(action.equals(ComService.ACTION_GET_PUS_TIME)){
                String info = intent.getExtras().getString("string"); 
                StringBuffer subBuf=new StringBuffer();
                subBuf.append(info);

                cpuTime.setText("Board  Time: " + "20" + subBuf.substring(20, 22) + "-" +subBuf.substring(22, 24) + "-" + subBuf.substring(24, 26) + " " + 
                                               subBuf.substring(28, 30) + ":" +subBuf.substring(30, 32) + ":" + subBuf.substring(32, 34));



            } else if(action.equals(ComService.ACTION_SET_PUS_TIME)){
                
            } else if (action.equals(ComService.ACTION_FINISH)) {
                Log.e("chengyake", "in PUSTimeActivity finish");
                finish();
            }
        }

    };


    /**Called when the activity is first created.*/
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.pus_time);

        initializeViews();
        updateTime();

		IntentFilter myIntentFilter = new IntentFilter();
		myIntentFilter.addAction(ComService.ACTION_GET_PUS_TIME);
		myIntentFilter.addAction(ComService.ACTION_SET_PUS_TIME);
		myIntentFilter.addAction(ComService.ACTION_FINISH);
		registerReceiver(mBroadcastReceiver, myIntentFilter);

        Intent j  = new Intent();  
        j.setClass(PUSTimeActivity.this, ComService.class);
        getApplicationContext().bindService(j, mServiceConnection, BIND_AUTO_CREATE);

        new TimeThread().start();
    }

    @Override
    protected void onDestroy() {
        Log.e("chengyake", "Override in PUSTimeActivity onDestroy func");
        getApplicationContext().unbindService(mServiceConnection);
        this.unregisterReceiver(mBroadcastReceiver);
        super.onDestroy();
    }

    private void initializeViews(){
        sysTime = (TextView)findViewById(R.id.systime);
        cpuTime = (TextView)findViewById(R.id.cputime);
        
        uploadTime = (Button) findViewById(R.id.upload_time);
        downloadTime = (Button)findViewById(R.id.download_time);

        uploadTime.setOnClickListener(new View.OnClickListener() {
           @Override
           public void onClick(View v) {

                try {
                    mIComService.getPUSTime();
                } catch  (Exception e) {
                    throw new IllegalStateException("upload error", e);
                }                

           }
   
        });

        downloadTime.setOnClickListener(new View.OnClickListener() {
           @Override
           public void onClick(View v) {

           new AlertDialog.Builder(PUSTimeActivity.this)
            .setTitle("确定下载？")
            .setIcon(android.R.drawable.ic_dialog_info)
            .setPositiveButton("确定", new DialogInterface.OnClickListener() { 
                @Override  
                public void onClick(DialogInterface dia, int which) {  

                    Calendar c = Calendar.getInstance();
                    int week = c.get(Calendar.DAY_OF_WEEK); 
                    long date = System.currentTimeMillis();
                    CharSequence sysDateStr = DateFormat.format("yyMMdd", date);
                    CharSequence sysTimeStr = DateFormat.format("HHmmss", date);

                    String time = sysDateStr + "0" + week + sysTimeStr;

                    try {
                        mIComService.setPUSTime(time);
                    } catch  (Exception e) {
                        throw new IllegalStateException("upload error", e);
                    }
                }

            })
            .setNegativeButton("取消", null)
            .show();
           }

        });
    }

    private void updateTime() {

        long time = System.currentTimeMillis();
        CharSequence sysTimeStr = DateFormat.format("yyyy-MM-dd HH:mm:ss", time);
        sysTime.setText("Mobile Time: " + sysTimeStr);
    }


}
