package com.android.bshadow;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.os.Handler;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.util.Log;
import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;

public class BshadowService extends Service  {

    private static final String TAG = "BshadowService";  
	private static final long SCANNING_TIMEOUT = 5 * 1000; /* 5 seconds */

    static final String STARTSCAN = "com.android.bshadow.startScan";  
    static final String STOPSCAN = "com.android.bshadow.stopScan";  
    static final String SACNRET = "com.android.bshadow.scanRet";  
    static final String CONNECTSTAT = "com.android.bshadow.connectStat";


	private boolean mScanning = false;
	private Handler mHandler = new Handler();
    private BshadowReceiver mReceiver;
    private BleWrapper mBleWrapper = null;
    private final HashMap<String, BluetoothDevice> mDevices = new HashMap<String, BluetoothDevice>();//Address  && device

    @Override
    public void onCreate() {
        super.onCreate();

        mBleWrapper = new BleWrapper(this, new BleWrapperUiCallbacks.Null() {
            //......

        	@Override
        	public void uiDeviceFound(final BluetoothDevice device, final int rssi, final byte[] record) {
        		handleFoundDevice(device, rssi, record);
        	}

        });
		if(mBleWrapper.initialize() == false) {
			onDestroy();
		}

        doRegisterReceiver();
    }

    @Override  
    public void onStart(Intent intent, int startId)  
    { 
        super.onStart(intent, startId);  
        Log.i(TAG, "onStart");  
    } 


    @Override  
    public void onDestroy()  
    { 
        super.onDestroy();  
        Log.i(TAG, "onDestroy");  
    } 

    @Override  
    public IBinder onBind(Intent intent)  
    {  
        return null;  
    }  



    private void doRegisterReceiver() {
        mReceiver=new BshadowReceiver();
        IntentFilter filter = new IntentFilter(STARTSCAN);
        filter.addAction(STOPSCAN);
        registerReceiver(mReceiver, filter);
    }


    public class BshadowReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i(TAG, "onReceive");  
            String action = intent.getAction();
            if (action.equals(STARTSCAN)) {
                //onDestroy();
                mScanning = true;
                addScanningTimeout();    	
                mBleWrapper.startScanning();
                Log.i(TAG, "start scan....");
            }

            if (action.equals(STOPSCAN)) {
                //test: send data to board
                byte[] nData = new byte[]{55,56,57,58};
                mBleWrapper.writeDataToCharacteristic(nData);
            }
    
        }
    }


    protected void sendContentBroadcast(String name) {
        Intent intent=new Intent();
        intent.setAction("com.example.servicecallback.content");
        intent.putExtra("name", name);
        sendBroadcast(intent);
    }

    private void handleFoundDevice(final BluetoothDevice device,
            final int rssi,
            final byte[] scanRecord)
	{
        //mDevicesListAdapter.addDevice(device, rssi, scanRecord);
        //mDevicesListAdapter.notifyDataSetChanged();
        Log.i(TAG, "name: " + device.getName() + " rssi: ");
        mDevices.put(device.getAddress(), device);
	}	

	private void addScanningTimeout() {
		Runnable timeout = new Runnable() {
            @Override
            public void run() {
            	if(mBleWrapper == null) return;
                mScanning = false;
                mBleWrapper.stopScanning();
                Log.i(TAG, "stop scan....");
                //invalidateOptionsMenu();
                connectToDevice();
            }
        };
        mHandler.postDelayed(timeout, SCANNING_TIMEOUT);
	}    

    private void connectToDevice()
    {
        Iterator iter = mDevices.entrySet().iterator();
        while (iter.hasNext()) {
            Map.Entry entry = (Map.Entry) iter.next();
            BluetoothDevice val = (BluetoothDevice)entry.getValue();
            if(val.getName().equals("Quintic BLE")) {
                connectDevice(val);
            }
        }
    }

    private void connectDevice(final BluetoothDevice device)
	{
        Log.i(TAG, "name: " + device.getName() + " address: " + device.getAddress());
    	mBleWrapper.connect(device.getAddress());
	}	

}



