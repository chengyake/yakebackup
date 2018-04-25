package com.android.cheng;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.content.Context;
import android.text.InputType;
import android.util.Log;
import java.util.ArrayList;
import java.util.List;
import java.io.IOException;
import android.graphics.Color;

import android.widget.Toast;
import android.os.IBinder;
import android.content.ServiceConnection;

import android.content.ComponentName;
import android.os.SystemClock;
import android.view.View.OnFocusChangeListener;


import android.content.Intent;
import android.content.IntentFilter;  

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.content.BroadcastReceiver;  
import android.content.DialogInterface;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;

import android.content.SharedPreferences;

import android.app.ProgressDialog;
public class PUSCLPWActivity extends Activity {
    


    private IComService mIComService;
	private Button clpw;
	private TextView times;


    private ServiceConnection mServiceConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName name, IBinder service) {  
            mIComService = IComService.Stub.asInterface(service);
        } 

        public void onServiceDisconnected(ComponentName name) {  
        }  
    }; 

/*************************************************/
   ProgressDialog dialog; 
   class CLPWThread extends Thread {
       public void run() {

           Intent intent1 = new Intent();
           intent1.setAction("clpw_show_dialog");
           sendBroadcast(intent1);



            clpw.setClickable(false);
                  try {
                     if(! ComService.UART_PUS.equals(mIComService.getUartStatus())) {
                         clpw.setClickable(true);
                         Toast.makeText(getApplicationContext(), "UART Error",
                             Toast.LENGTH_SHORT).show();
                             return;
                     }
                     while(0!=mIComService.downloadPUSConsole("84020000", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSConsole("84020001", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSConsole("84020002", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSConsole("84020003", "00")) {
                          SystemClock.sleep(10);
                     }

                     while(0!=mIComService.preDownloadPUSJobdata("68")) {
                          SystemClock.sleep(200);
                     }

                     while(0!=mIComService.downloadPUSJobdata("0061F019", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F01A", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F01B", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F01C", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F01D", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F01E", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F01F", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F020", "19")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F021", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F022", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F023", "00")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.downloadPUSJobdata("0061F024", "1E")) {
                          SystemClock.sleep(10);
                     }
                     while(0!=mIComService.flashPUSJobdata()) {
                          SystemClock.sleep(200);
                     }
                     //Toast.makeText(getApplicationContext(), "Clear PW Success",
                     //  Toast.LENGTH_SHORT).show();

              
                         } catch  (Exception e) {
                             throw new IllegalStateException("getVersionInfo error", e);
                         }

                     clpw.setClickable(true);

                     Intent intent = new Intent();
                     intent.setAction("clpw_cancel_dialog");
                     sendBroadcast(intent);


       }
   }

/*************************************************/








    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver(){
        @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action.equals(ComService.ACTION_FINISH)) {
                    finish();
                } else if(action.equals("clpw_show_dialog") ) {

                    dialog = new ProgressDialog(PUSCLPWActivity.this); 
                    dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER); 
                    dialog.setTitle("PUS"); 
                    dialog.setMessage("正在清除数据..."); 
                    dialog.setIcon(android.R.drawable.ic_dialog_map); 
                    dialog.setIndeterminate(false); 
                    dialog.setCancelable(false);
                    dialog.show(); 
                } else if(action.equals("clpw_cancel_dialog") ) {
                     dialog.cancel(); 
                }
            }
    };

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		
		setContentView(R.layout.pus_clpw);

		IntentFilter myIntentFilter = new IntentFilter();
		myIntentFilter.addAction(ComService.ACTION_FINISH);
		myIntentFilter.addAction("clpw_show_dialog");
		myIntentFilter.addAction("clpw_cancel_dialog");
		registerReceiver(mBroadcastReceiver, myIntentFilter);

        Intent j  = new Intent();  
        j.setClass(this, ComService.class);
        getApplicationContext().bindService(j, mServiceConnection, BIND_AUTO_CREATE);
        


        times=(TextView)findViewById(R.id.times);
        clpw=(Button)findViewById(R.id.clpw);
        clpw.setOnClickListener(new View.OnClickListener() {
	    public void onClick(View v) {

            new AlertDialog.Builder(PUSCLPWActivity.this)
            .setTitle("确定清除？")
            .setIcon(android.R.drawable.ic_dialog_info)
            .setPositiveButton("确定", new DialogInterface.OnClickListener() { 
                @Override  
                public void onClick(DialogInterface dia, int which) {

                    CLPWThread mCLPWThread = new CLPWThread();
                    mCLPWThread.start();
                    SharedPreferences shareData = getSharedPreferences("sunupdata", 0); 
                         SharedPreferences.Editor shareEditor = shareData.edit(); 
                         int t = shareData.getInt("sunup_clpw", 0);
                         if(t>0) {
                             t--;
                         } else {
                             t=0;
                             clpw.setVisibility(View.INVISIBLE);
                         }
                         shareEditor.putInt("sunup_clpw", t); 
                         shareEditor.commit();

                         times.setText("" + t);

                }
            })
            .setNegativeButton("取消",  new DialogInterface.OnClickListener() { 
                @Override  
                public void onClick(DialogInterface dia, int which) {  
                }
            })
            .show();


	                   }
		});
        
        //shang ... button
        SharedPreferences shareData = getSharedPreferences("sunupdata", 0); 
        int t = shareData.getInt("sunup_clpw", 0);
        if(t<=0) {
            clpw.setVisibility(View.INVISIBLE);
        }

        times.setText("" + t);

	}

    @Override
    protected void onDestroy() {
        getApplicationContext().unbindService(mServiceConnection);
        this.unregisterReceiver(mBroadcastReceiver);
        super.onDestroy();
    }


}



