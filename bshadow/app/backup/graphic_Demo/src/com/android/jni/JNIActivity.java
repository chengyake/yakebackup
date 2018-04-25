package com.android.jni;

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

import java.io.IOException;

public class JNIActivity extends Activity{

    private final String TAG = "chengyake";
	private Button mBack = null;
    Graphic myGraphic=null;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
		mBack = (Button) findViewById(R.id.back);
        myGraphic=new Graphic(this, null);

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
