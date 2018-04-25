package com.android.cheng;

import android.widget.TextView;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.TabActivity;
import android.content.DialogInterface;
import android.graphics.Color;
import android.os.Bundle;
import android.widget.TabHost;
import android.widget.Toast;
import android.widget.TabHost.OnTabChangeListener;
import android.content.res.Resources;
import android.content.Intent;
import android.view.KeyEvent;
import android.widget.TabWidget;
import java.io.IOException;
import android.os.RemoteException;
import android.content.BroadcastReceiver;  
import android.content.Intent;
import android.content.IntentFilter; 
import android.content.Context;
import android.util.Log;
import android.widget.Toast;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;

import android.os.SystemClock;
import android.os.IBinder;
import android.content.ServiceConnection;
import android.content.ComponentName;

public class MainActivity extends TabActivity {


    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver(){
        @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action.equals(ComService.ACTION_DIALOG)) {
                    Log.e("chengyake", "in MainActivity finish");

                    Toast.makeText(getApplicationContext(), "Connect UART Please",
                            Toast.LENGTH_SHORT).show();
                } else if(action.equals(ComService.ACTION_FINISH)) {
                    //Intent mIntent = new Intent();
                    //mIntent.setAction(ComService.ACTION_FINISH);
                    //sendBroadcast(mIntent);
                    finish();
                }
            }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);

        Log.e("chengyake", "Override in MainActivity onCreate func");

        Intent i  = new Intent();  
        i.setClass(MainActivity.this, ComService.class);  
        getApplicationContext().startService(i);

		IntentFilter myIntentFilter = new IntentFilter();
		myIntentFilter.addAction(ComService.ACTION_FINISH);
		myIntentFilter.addAction(ComService.ACTION_DIALOG);
		registerReceiver(mBroadcastReceiver, myIntentFilter);

        Resources res = getResources(); 
        TabHost tabHost = getTabHost();
        TabWidget tabWidget = tabHost.getTabWidget();
        TabHost.TabSpec spec;
        Intent intent;

        intent = new Intent().setClass(this, PUSActivity.class);

        spec = tabHost.newTabSpec("a").setIndicator("PUS",
                null).setContent(intent);
        tabHost.addTab(spec);

        intent = new Intent().setClass(this, PUMActivity.class);
        spec = tabHost.newTabSpec("b").setIndicator("PUM",
                null).setContent(intent);
        tabHost.addTab(spec);

        for (int j =0; j < tabWidget.getChildCount(); j++) {
            //tabWidget.getChildAt(j).getLayoutParams().height = 160;
            //tabWidget.getChildAt(j).getLayoutParams().width = 125;
            TextView tv = (TextView) tabWidget.getChildAt(j).findViewById(android.R.id.title);
            tv.setTextSize(20);
            //tv.setGravity(0);
            tv.setTextColor(this.getResources().getColorStateList(android.R.color.white));
            //tabWidget.setBackgroundResource(R.drawable.background);
        }

        tabHost.setCurrentTab(0);

    }

/*
    @Override
    public void onBackPressed() {
    //public boolean onKeyDown(int keyCode, KeyEvent event) {
        Log.e("chengyake", "+++++++++++++++++++++");
	    new AlertDialog.Builder(MainActivity.this).setTitle("确认退出吗？")
		.setIcon(android.R.drawable.ic_dialog_info)
		.setPositiveButton("确定", new DialogInterface.OnClickListener() {
		    @Override
		    public void onClick(DialogInterface dialog, int which) {
            Log.e("chengyake", "++++1+++++++++++++++++");
			    //MainActivity.this.finish();
			    MainActivity.this.onDestroy();
		    }
		})
		.setNegativeButton("返回", new DialogInterface.OnClickListener() {
		    @Override
		    public void onClick(DialogInterface dialog, int which) {
		    }
		}).show();

        //super.onBackPressed();
        Log.e("chengyake", "++++2+++++++++++++++++");
        //return true;
    }
*/

    @Override
    protected void onDestroy() {

        Intent i  = new Intent();  
        i.setClass(MainActivity.this, ComService.class);  
        getApplicationContext().stopService(i);
        MainActivity.this.unregisterReceiver(mBroadcastReceiver);
        MainActivity.super.onDestroy();
    }
}

