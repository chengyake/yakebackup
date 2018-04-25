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

public class JNIActivity extends Activity implements OnClickListener{

    private final String TAG = "chengyake";

	private static final int ENABLE_BT_REQUEST_ID = 1;
    private static final int MESSAGE_REFRESH = 101;
    private static final long REFRESH_TIMEOUT_MILLIS = 1000;

	private EditText mInputText = null;
	private Button mCommit = null;
	private TextView mShowText = null;

	private BluetoothAdapter mBluetoothAdapter;
	private DeviceListAdapter mLeDeviceListAdapter;

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_REFRESH:
                    mHandler.sendEmptyMessageDelayed(MESSAGE_REFRESH, REFRESH_TIMEOUT_MILLIS);
                    Log.e(TAG, "------refresh-----");
                    break;
                default:
                    super.handleMessage(msg);
                    break;
            }
        }

    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.main);
        
        initView();
    }
    @Override
    protected void onResume() {
        super.onResume();
        //mHandler.sendEmptyMessage(MESSAGE_REFRESH);
    }

    @Override
        protected void onPause() {
            super.onPause();
            //mHandler.removeMessages(MESSAGE_REFRESH);
        }

	private void initView() {
		mInputText = (EditText) findViewById(R.id.inputText);
		mCommit = (Button) findViewById(R.id.commit);
		mCommit.setOnClickListener(this);
		mShowText = (TextView) findViewById(R.id.showText);
		
	}

	public void onClick(View v) {
		switch (v.getId()) {
		case R.id.commit:
            Log.e(TAG, "------click-----");

            //1.support le?            always avilable; no effect
            if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
                Toast.makeText(this, "no  android.hardware.bluetooth_le", Toast.LENGTH_SHORT).show();
                Log.e(TAG, "------exit-----");
                finish();
            }

            final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
            mBluetoothAdapter = bluetoothManager.getAdapter();
    
            //2.open bt
            if (mBluetoothAdapter == null || !mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, ENABLE_BT_REQUEST_ID);
            }

            mBluetoothAdapter.stopLeScan(mLeScanCallback);
            mBluetoothAdapter.startLeScan(mLeScanCallback);

            break;

		default:
			break;
		}
	}

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // user didn't want to turn on BT
        if (requestCode == ENABLE_BT_REQUEST_ID) {
        	if(resultCode == Activity.RESULT_CANCELED) {
		    	btDisabled();
                Log.e(TAG, "------user exit-----");
		        return;
		    }
        }
        super.onActivityResult(requestCode, resultCode, data);
    }

    private void btDisabled() {
    	Toast.makeText(this, "Sorry, BT has to be turned ON for us to work!", Toast.LENGTH_LONG).show();
    	finish();
    }
    


    private BluetoothAdapter.LeScanCallback mLeScanCallback =
        new BluetoothAdapter.LeScanCallback() {
            @Override
            public void onLeScan(final BluetoothDevice device, final int rssi, final byte[] scanRecord) {
                runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            //mLeDeviceListAdapter.addDevice(device, rssi, scanRecord);
                            //mLeDeviceListAdapter.notifyDataSetChanged();
                            Log.e(TAG, "name:" + device.getName() + "rssi:" + rssi);
                        }
                });
            }
        };

}
