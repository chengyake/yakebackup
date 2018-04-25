package com.android.bshadow;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.UUID;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.util.Log;

public class BleWrapper {

    private static final String TAG = "BshadowBleWrapper";  
    private static final byte[] notifyData = new byte[]{1};

	private Service mParent = null;    
	private boolean mConnected = false;
	private String mDeviceAddress = "";

    private BluetoothManager mBluetoothManager = null;
    private BluetoothAdapter mBluetoothAdapter = null;
    private BluetoothDevice  mBluetoothDevice = null;
    private BluetoothGatt    mBluetoothGatt = null;
    private BluetoothGattService mBluetoothSelectedService = null;
    private BluetoothGattCharacteristic mWriteCharacteristic = null;
    private List<BluetoothGattService> mBluetoothGattServices = null;	
    private Object[] mDescriptorList = new BluetoothGattDescriptor[7];
    private int mDescriptorIdx = 0;

    private BleWrapperUiCallbacks mUiCallback = null;
    private static final BleWrapperUiCallbacks NULL_CALLBACK = new BleWrapperUiCallbacks.Null(); 
    
    public BleWrapper(Service parent, BleWrapperUiCallbacks callback) {
    	this.mParent = parent;
    	mUiCallback = callback;
    	if(mUiCallback == null) mUiCallback = NULL_CALLBACK;
    }

    public BluetoothManager           getManager() { return mBluetoothManager; }
    public BluetoothAdapter           getAdapter() { return mBluetoothAdapter; }
    public BluetoothDevice            getDevice()  { return mBluetoothDevice; }
    public BluetoothGatt              getGatt()    { return mBluetoothGatt; }
    public BluetoothGattService       getCachedService() { return mBluetoothSelectedService; }
    public List<BluetoothGattService> getCachedServices() { return mBluetoothGattServices; }
    public boolean                    isConnected() { return mConnected; }

	public boolean checkBleHardwareAvailable() {
		final BluetoothManager manager = (BluetoothManager) mParent.getSystemService(Context.BLUETOOTH_SERVICE);
		if(manager == null) return false;
		final BluetoothAdapter adapter = manager.getAdapter();
		if(adapter == null) return false;
		
		boolean hasBle = mParent.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE);
		return hasBle;
	}

	public boolean isBtEnabled() {
		final BluetoothManager manager = (BluetoothManager) mParent.getSystemService(Context.BLUETOOTH_SERVICE);
		if(manager == null) return false;
		
		final BluetoothAdapter adapter = manager.getAdapter();
		if(adapter == null) return false;
		
		return adapter.isEnabled();
	}
	
	public void startScanning() {
        mBluetoothAdapter.startLeScan(mDeviceFoundCallback);
	}
	
	public void stopScanning() {
		mBluetoothAdapter.stopLeScan(mDeviceFoundCallback);	
	}
	
    public boolean initialize() {
        if (mBluetoothManager == null) {
            mBluetoothManager = (BluetoothManager) mParent.getSystemService(Context.BLUETOOTH_SERVICE);
            if (mBluetoothManager == null) {
                return false;
            }
        }

        if(mBluetoothAdapter == null) mBluetoothAdapter = mBluetoothManager.getAdapter();
        if (mBluetoothAdapter == null) {
            return false;
        }
        return true;    	
    }

    public boolean connect(final String deviceAddress) {
        if (mBluetoothAdapter == null || deviceAddress == null) return false;
        mDeviceAddress = deviceAddress;
        
        if(mBluetoothGatt != null && mBluetoothGatt.getDevice().getAddress().equals(deviceAddress)) {
        	return mBluetoothGatt.connect();
        }
        else {
            mBluetoothDevice = mBluetoothAdapter.getRemoteDevice(mDeviceAddress);
            if (mBluetoothDevice == null) {
                return false;
            }
        	mBluetoothGatt = mBluetoothDevice.connectGatt(mParent, false, mBleCallback);
        }
        return true;
    }  
    
    public void diconnect() {
    	if(mBluetoothGatt != null) mBluetoothGatt.disconnect();
    	 mUiCallback.uiDeviceDisconnected(mBluetoothGatt, mBluetoothDevice);
    }

    public void close() {
    	if(mBluetoothGatt != null) mBluetoothGatt.close();
    	mBluetoothGatt = null;
    }

    public void startServicesDiscovery() {

        Log.e(TAG, "startServiceDiscovery..");
    	if(mBluetoothGatt != null) mBluetoothGatt.discoverServices();
    }
    
    public void getQPPSService() {
        Log.e(TAG, "getQPPSService..");
        for(BluetoothGattService svc : mBluetoothGattServices) {
        	Log.e(TAG, "service uuid: " + svc.getUuid().toString() );
            if (svc.getUuid().equals(BleDefinedUUIDs.Service.QPPS)) {
                mBluetoothSelectedService = svc;
                return;
            }
        }
    }
    
    public void setQPPSCharateristics() {

        int i=0;

        Log.e(TAG, "setQPPSNotify 0..");
        if(mBluetoothSelectedService==null) {
            Log.e(TAG, "service is null ... ");
            return;
        }

        List<BluetoothGattCharacteristic> charList = mBluetoothSelectedService.getCharacteristics();
        for(BluetoothGattCharacteristic characteristic:charList) {

            if((BluetoothGattCharacteristic.PROPERTY_WRITE & characteristic.getProperties()) != 0){//find write characteristic
                mWriteCharacteristic = characteristic;
                Log.e(TAG, "setQPPSNotify w..");
            } else {
                List<BluetoothGattDescriptor> list = characteristic.getDescriptors();
                if(list.size()>0){
                    mDescriptorList[i++]=list.get(0);
                }
                Log.e(TAG, "setQPPSNotify n..");
                //((BluetoothGatt)mBluetoothGatt).setCharacteristicNotification(characteristic, true);
                setNotificationForCharacteristic(characteristic, true);
            }
        }

        //set first Descriptor notify
        mDescriptorIdx=0;
        writeDescriptor();


    }

    private void writeDescriptor(){
    	if(mDescriptorIdx<7) {
           ((BluetoothGattDescriptor)mDescriptorList[mDescriptorIdx]).setValue(notifyData);
            mBluetoothGatt.writeDescriptor(((BluetoothGattDescriptor)mDescriptorList[mDescriptorIdx]));
            mDescriptorIdx++;
        } else {
            Log.e(TAG, "7 char notify over!");
        }
    }

    public void getSupportedServices() {
        Log.e(TAG, "getSupportedService..");
    	if(mBluetoothGattServices != null && mBluetoothGattServices.size() > 0) mBluetoothGattServices.clear();
        if(mBluetoothGatt != null) mBluetoothGattServices = mBluetoothGatt.getServices();

    }


    public void requestCharacteristicValue(BluetoothGattCharacteristic ch) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) return;
        
        mBluetoothGatt.readCharacteristic(ch);
    }

    /* get characteristic's value (and parse it for some types of characteristics) 
     * before calling this You should always update the value by calling requestCharacteristicValue() */
    public void getCharacteristicValue(BluetoothGattCharacteristic ch) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null || ch == null) return;
        
        UUID uuid = ch.getUuid();
        byte[] rawValue = ch.getValue();

        Log.e(TAG, "getCharaUUID: " + uuid + "data[0]: " +rawValue[0] );

        if(uuid.equals(BleDefinedUUIDs.Characteristic.WS1)) {


        }

    }    
    


    public void writeDataToCharacteristic(final byte[] dataToWrite) {
    	BluetoothGattCharacteristic ch = mWriteCharacteristic;
    	if (mBluetoothAdapter == null || mBluetoothGatt == null || ch == null) return;
    	
    	ch.setValue(dataToWrite);
    	mBluetoothGatt.writeCharacteristic(ch);
    }
    


    public void setNotificationForCharacteristic(BluetoothGattCharacteristic ch, boolean enabled) {
        if (mBluetoothAdapter == null || mBluetoothGatt == null) return;
        
        boolean success = mBluetoothGatt.setCharacteristicNotification(ch, enabled);
        if(!success) {
        	Log.e(TAG, "Seting proper notification status for characteristic failed!");
        }
        
        // This is also sometimes required (e.g. for heart rate monitors) to enable notifications/indications
        // see: https://developer.bluetooth.org/gatt/descriptors/Pages/DescriptorViewer.aspx?u=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
        BluetoothGattDescriptor descriptor = ch.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
        if(descriptor != null) {
        	byte[] val = enabled ? BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE : BluetoothGattDescriptor.DISABLE_NOTIFICATION_VALUE;
	        descriptor.setValue(val);
	        mBluetoothGatt.writeDescriptor(descriptor);
        }
    }
    
    private BluetoothAdapter.LeScanCallback mDeviceFoundCallback = new BluetoothAdapter.LeScanCallback() {
        @Override
        public void onLeScan(final BluetoothDevice device, final int rssi, final byte[] scanRecord) {//fixd api params 
        	mUiCallback.uiDeviceFound(device, rssi, scanRecord);
        }
    }; 
    

    private final BluetoothGattCallback mBleCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
            	mConnected = true;
            	startServicesDiscovery();
            }
            else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
            	mConnected = false;
            }
            Log.e(TAG, "onConnectionStateChange: status = " + mConnected);
        }


        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
            	getSupportedServices();
            	getQPPSService();
                setQPPSCharateristics();
            }
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                                         BluetoothGattCharacteristic characteristic,
                                         int status)
        {
            Log.e(TAG, "onCharacteristicRead...");
            if (status == BluetoothGatt.GATT_SUCCESS) {
            	getCharacteristicValue(characteristic);
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                                            BluetoothGattCharacteristic characteristic)
        {
            Log.e(TAG, "onCharacteristicChanged...");
        	getCharacteristicValue(characteristic);
        }
        
        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
            Log.e(TAG, "onCharacteristicWrite...");
        };
        
        @Override
        public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
            Log.e(TAG, "onReadRssi...");
        };

        @Override
        public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status){
            Log.e(TAG, "onDescriptorRead...");
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status){
            Log.e(TAG, "onDescriptorWrite...");
            writeDescriptor();
        }

    };
    

    
}
