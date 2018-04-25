package com.android.bshadow;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.content.Context;
import android.content.Intent;


import android.content.pm.PackageManager;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothDevice;

import android.util.Log;
import java.util.ArrayList;
import java.util.List;
import android.content.BroadcastReceiver;
import java.io.IOException;
import android.app.Dialog;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;

public class LoginActivity extends Activity {

    private final String TAG = "BshadowLoginActivity";
	private Button mBack = null;
	private Button mLogin = null;
	private Button mTips = null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.login);
		mBack = (Button) findViewById(R.id.back);
        mBack.setOnClickListener(new View.OnClickListener() {
            @Override  
            public void onClick(View v) {  
                Log.i(TAG, "back button onClick");  
                new AlertDialog.Builder(LoginActivity.this, AlertDialog.THEME_HOLO_LIGHT)
                .setTitle("title")
                .setMessage("content")
                .setPositiveButton(R.string.yes,
                    new DialogInterface.OnClickListener() {
                         @Override
                         public void onClick(DialogInterface dialog, int which) {
                             finish();
                         }
                     })
                .setNegativeButton(R.string.no,
                     new DialogInterface.OnClickListener() {
                         @Override
                         public void onClick(DialogInterface dialog, int which) {
                             dialog.cancel();
                         }
                     })

                .create()

                .show();

            }  
        });  
		mLogin = (Button) findViewById(R.id.login);
        mLogin.setOnClickListener(new View.OnClickListener() { 
            @Override  
            public void onClick(View v) {  
                Log.i(TAG, "login button onClick");  
                //Intent it = new Intent(LoginActivity.this, ConnectActivity.class);
                //startActivity(it);  
                Intent intent=new Intent();
                intent.setAction(BshadowService.STARTSCAN);
                sendBroadcast(intent);
            }  
        });  
		mTips = (Button) findViewById(R.id.tips);
        mTips.setOnClickListener(new View.OnClickListener() { 
            @Override  
            public void onClick(View v) {  
                Log.i(TAG, "tips button onClick");  
                //Intent it = new Intent(LoginActivity.this, ConnectActivity.class);
                //startActivity(it);  
                Intent intent=new Intent();
                intent.setAction(BshadowService.STOPSCAN);
                sendBroadcast(intent);
            }  
        }); 


        Intent intent = new Intent(LoginActivity.this, BshadowService.class); 
        startService(intent);
    }


    @Override
    protected void onResume() {
        super.onResume();
    }



    @Override
    protected void onPause() {
        super.onPause();
    }

}
