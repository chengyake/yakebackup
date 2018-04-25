package com.android.cheng;

import com.android.cheng.CodeInfo;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.text.format.Time;
import android.util.Log;
import android.os.RemoteException;
import java.lang.ref.WeakReference;

import android.content.BroadcastReceiver;  
import android.os.Handler;
import android.os.Message;
import com.android.cheng.driver.UsbSerialDriver;
import com.android.cheng.driver.UsbSerialProber;
import com.android.cheng.util.HexDump;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;

import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbManager;

import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import java.lang.String;
import android.widget.Toast;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import android.app.NotificationManager;
import android.app.Notification;
import android.app.PendingIntent;

import android.content.IntentFilter;  
import android.os.SystemClock;

public class ComService extends Service {
    private static final String TAG = "chengyake ComService";

    private static final boolean debug = true;

    private static final int READ_WAIT_MILLIS = 1000;
    private static final int BUFSIZ = 4096;

    static final String UART_UNCONNECTED = "com.android.cheng.uart.unconnect";
    static final String UART_CONNECTED = "com.android.cheng.uart.connect";
    static final String UART_INIT = "com.android.cheng.uart.init";
    static final String UART_HANDSHAKED = "com.android.cheng.uart.handshaked";
    static final String UART_PUS = "com.android.cheng.uart.pus";
    static final String UART_PUM = "com.android.cheng.uart.pum";


    static final String ACTION_BEATS = "com.android.cheng.action.BEATS";
    static final String ACTION_UART_HANDSHAKE = "com.android.cheng.action.UART_HANDSHAKE";
    static final String ACTION_GET_INFO = "com.android.cheng.action.GET_INFO";
    static final String ACTION_INTO_PUS = "com.android.cheng.action.INTO_PUS";
    static final String ACTION_EXIT_PUS = "com.android.cheng.action.EXIT_PUS";
    static final String ACTION_INTO_PUM = "com.android.cheng.action.INTO_PUM";
    static final String ACTION_EXIT_PUM = "com.android.cheng.action.EXIT_PUM";
    static final String ACTION_UPLOAD_PUS_CONSOLE  = "com.android.cheng.action.UPLOAD_PUS_CONSOLE";
    static final String ACTION_DOWNLOAD_PUS_CONSOLE  = "com.android.cheng.action.DOWNLOAD_PUS_CONSOLE";
    static final String ACTION_GET_PUS_ERROR = "com.android.cheng.action.GET_PUS_ERROR";
    static final String ACTION_DEL_PUS_ERROR = "com.android.cheng.action.DEL_PUS_ERROR";
    static final String ACTION_UPLOAD_PUS_JOBDATA = "com.android.cheng.action.UPLOAD_PUS_JOBDATA";
    static final String ACTION_DOWNLOAD_PUS_JOBDATA = "com.android.cheng.action.DOWNLOAD_PUS_JOBDATA";
    static final String ACTION_FLASH_PUS_JOBDATA = "com.android.cheng.action.FLASH_PUS_JOBDATA";
    static final String ACTION_GET_PUS_TIME = "com.android.cheng.action.GET_PUS_TIME";
    static final String ACTION_SET_PUS_TIME = "com.android.cheng.action.SET_PUS_TIME";
    static final String ACTION_UPLOAD_PUM_CONSOLE = "com.android.cheng.action.UPLOAD_PUM_CONSOLE";
    static final String ACTION_DOWNLOAD_PUM_CONSOLE = "com.android.cheng.action.DOWNLOAD_PUM_CONSOLE";
    static final String ACTION_GET_PUM_ERROR = "com.android.cheng.action.GET_PUM_ERROR";
    static final String ACTION_DEL_PUM_ERROR = "com.android.cheng.action.DEL_PUM_ERROR";
    static final String ACTION_UPLOAD_PUM_JOBDATA = "com.android.cheng.action.UPLOAD_PUM_JOBDATA";
    static final String ACTION_DOWNLOAD_PUM_JOBDATA = "com.android.cheng.action.DOWNLOAD_PUM_JOBDATA";
    static final String ACTION_FLASH_PUM_JOBDATA = "com.android.cheng.action.DOWNLOAD_PUM_JOBDATA";
    static final String ACTION_FINISH = "com.android.cheng.action.FINISH";
    static final String ACTION_DIALOG = "com.android.cheng.action.DIALOG";
    static final String ACTION_DEBUG = "com.android.cheng.action.DEBUG";


    private static final int MESSAGE_DEBUG = 0;
    private static final int MESSAGE_BEATS = 1;
    private static final int MESSAGE_DIALOG = 2;
    private static final int MESSAGE_FINISH = 3;
    private static String uart_action;
    private static String uart_string;
    private static boolean mutex=true;

    private static int bigData = 0;


    private UsbManager mUsbManager;
    private UsbSerialDriver mDriver;
    private String status;
    private static String cpuType=UART_PUS;
    ReadThread mReadThread;
    private static int isStart;
    private static int isExit; //tmp


    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver(){
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if(action.equals(ACTION_INTO_PUS)){
                cpuType=UART_PUS;
                //Log.e("chengyake", "into pus");
            } else if (action.equals(ACTION_INTO_PUM)) {
                cpuType=UART_PUM;
                //Log.e("chengyake", "into pum");
            }
        }

    };


    private Handler mHandler = new Handler() {

        public void dispatchMessage(Message msg) {
            if (MESSAGE_BEATS == msg.what) {
                mHandler.sendMessageDelayed(mHandler.obtainMessage(MESSAGE_BEATS), 1000);
            } else if (MESSAGE_DIALOG == msg.what) {
                Intent mIntent = new Intent();
                mIntent.setAction(ACTION_DIALOG);
                sendBroadcast(mIntent);
                mHandler.sendMessageDelayed(mHandler.obtainMessage(MESSAGE_FINISH), 1000);
            } else if (MESSAGE_FINISH == msg.what) {
                Intent mIntent = new Intent();
                mIntent.setAction(ACTION_FINISH);
                sendBroadcast(mIntent);
                onDestroy();
            }

        };  
    };

   class ReadThread extends Thread {

        public static final int STOP = 0;
        public static final int RUNNING = 1;

        private boolean busy= false;

        private String pre_cmd;
        private String cmd_detect;
        private String cmd;
        private String rsp;

        byte[] mReadBuffer = new byte[BUFSIZ];
        private int mState = STOP;

        public int getStatus() {
            return mState;
        }

        public boolean setCmd(String str) {
            if(busy == false)  {
                busy = true;
                pre_cmd = str;
                return true;
            }
            return false;
        }
        public String getCmd() {
            return cmd_detect;
        }

        public void stopThread() {
            mState = STOP;
        }

        public void run() {
            int len=0;
            mState = RUNNING;
            if(pre_cmd == null) {
                //Log.e(TAG, "Error cmd is null");
                return ;
            }

            while(true) {
                //check
                if(mState == STOP) break;

                if (pre_cmd != null) {
                    cmd=pre_cmd;
                    cmd_detect=pre_cmd;
                    pre_cmd=null;
                } 
                
                //write
                //Log.e(TAG, "-------------------------cmd: " + cmd);
                cmd="\2" + cmd + "\3";
                try {
                    int ret = mDriver.write(cmd.getBytes(), READ_WAIT_MILLIS);
                } catch (IOException e) {
                    //Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
                    //dialog && exit
                    //System.exit(0);
                    if(isExit==0) {
                        isExit = 1;
                    //Log.e("chengyake", "--------------------------------------------- in comservice finish");
                    Intent intent = new Intent();
                    intent.setAction(ACTION_FINISH);
                    sendBroadcast(intent);
                    SystemClock.sleep(1000);
                    System.exit(0);
                    }

                }

                //SystemClock.sleep(100);

                //read
                try {
                    len = mDriver.read(mReadBuffer, READ_WAIT_MILLIS);
                } catch (IOException e) {
                    //Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
                }
                


                //rongcuo
                if(len < 3) {
                    SystemClock.sleep(100);
                    //Log.e(TAG, "-------------------------len: " + len);
                    for(int i=0; i<len; i++) {
                        //Log.e(TAG, "-------------------------data: " + mReadBuffer[i]);
                    }
                    
                    try {
                        len = mDriver.read(mReadBuffer, READ_WAIT_MILLIS);
                    } catch (IOException e) {
                        //Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
                    }

                    StringBuffer cmdBuf=new StringBuffer(); 

                    int cmdLen = cmd.length();
                
                    cmdBuf.append(cmd);


                    cmd=cmdBuf.substring(1, cmdLen-1);


                    continue;

                }
                //beats
                if(mReadBuffer[1] == 0x21) {
                    if(busy == true) {
                        cmd=CodeInfo.BEATS;
                    } else {
                        SystemClock.sleep(100);
                        if(cpuType.equals(status)) {
                            cmd=CodeInfo.BEATS;
                        } else {
                            if(cpuType.equals(UART_PUS)) {
                                busy = true;
                                cmd = CodeInfo.EXIT_PUM_C;
                                uart_action=ACTION_INTO_PUS;
                            }
                            if(cpuType.equals(UART_PUM)) {
                                busy = true;
                                cmd = CodeInfo.EXIT_PUS_C;
                                uart_action=ACTION_INTO_PUM;
                            }
                        }
                    }//end else ! 0x21
                    //Log.e(TAG, "-------------------------rsp: beats");
                    continue;
                }


                StringBuffer rspBuf=new StringBuffer(); 
                rspBuf.append(new String(mReadBuffer));
                rsp=rspBuf.substring(1, len-1);
                //Log.e(TAG, "-------------------------rsp: " + rsp);

                String chk = detectCode(rsp);

                if(chk == null) {
                    //1->1
                    /*
                    if(rsp.length() > 150) {
                        StringBuffer subBuf=new StringBuffer();
                        subBuf.append(rsp);
                        rsp = subBuf.substring(0, 150);
                    }*/

                    Intent intent = new Intent();
                    intent.setAction(uart_action);
                    intent.putExtra("string", rsp);
                    sendBroadcast(intent);
                    cmd=CodeInfo.BEATS;

                    busy = false;
                } else {
                    //1->1 1->1
                    //1->111
                    //SystemClock.sleep(100);
                    cmd = chk;
                    cmd_detect = chk;
                }
            }
        }  
    }

    class ProbeThread extends Thread {

        public void run() {
            probeCp2102();
        }
    }

    private int probeCp2102() {
        int probe_times=0;

        mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        //find driver

        while(true) {
            mDriver = null;
            for (final UsbDevice device : mUsbManager.getDeviceList().values()) {
                final List<UsbSerialDriver> drivers =
                    UsbSerialProber.probeSingleDevice(mUsbManager, device);
                if (!drivers.isEmpty()) {
                    for (UsbSerialDriver driver : drivers) {
                        Log.d(TAG, "  + " + driver);
                        mDriver = driver;
                        //Log.e(TAG, "Found usb device: " + device + driver);
                    }
                } 
            }

            if(mDriver == null) {
                if(probe_times<1) {
                    probe_times++;
                    continue;
                }
                SystemClock.sleep(500);
                Intent intent = new Intent();
                intent.setAction(ACTION_FINISH);
                sendBroadcast(intent);
                SystemClock.sleep(1000);
                System.exit(0);
            } else {
                break;
            }

        }

        //open and set driver
        try {
            //Log.e(TAG, "Found usb device: debug");
            mDriver.open();
            mDriver.setParameters(115200, 8, UsbSerialDriver.STOPBITS_2, UsbSerialDriver.PARITY_ODD);
            mutex = false;
            //Log.e(TAG, "Found usb device: debug");
        } catch (IOException e) {
            //Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
            try {
                mDriver.close();
            } catch (IOException e2) {
                // Ignore.
            }
            mDriver = null;
            return -1;

        }

        //handshake
        //writeAsync(CodeInfo.UART_HANDSHAKE_1C);
        mReadThread = new ReadThread();
        mReadThread.setCmd(CodeInfo.UART_HANDSHAKE_1C);
        mReadThread.start();
        

        return 0;

     }

    @Override
    public IBinder onBind(Intent intent) {
        //Log.e(TAG, "Override start IBinder");
        return aBinder;
    }

    @Override
    public void onCreate() {
        //Log.e(TAG, "Override start onCreate");
        super.onCreate();

		IntentFilter myIntentFilter = new IntentFilter();
		myIntentFilter.addAction(ComService.ACTION_INTO_PUS);
		myIntentFilter.addAction(ComService.ACTION_INTO_PUM);
		registerReceiver(mBroadcastReceiver, myIntentFilter);

        new ProbeThread().start();


        //mHandler.sendMessageDelayed(mHandler.obtainMessage(MESSAGE_BEATS), 1000);
    }

    @Override
    public void onStart(Intent intent, int startId) {
        //Log.e(TAG, "Override start onStart~~~");
        super.onStart(intent, startId);

    }

    @Override
    public void onDestroy() {
        //Log.e(TAG, "Override start onDestroy~~~");
        if(mReadThread != null) {
            mReadThread.stopThread();
        }
        super.onDestroy();
    }

    @Override
    public boolean onUnbind(Intent intent) {
        //Log.e(TAG, "Override start onUnbind ");
        super.onUnbind(intent);
        //onDestroy();
        return true;
    }

    public String getSystemTime(){

        //Log.e(TAG, "chengyake start getSystemTime ");
        Time t = new Time();
        t.setToNow();
        return t.toString();
    }
    /*
    public int writeBeats(){
        
        //Log.e(TAG, "chengyake write Beats true");
        mutex = true;
        byte[] beats = {0x02, 0x21, 0x03};
        //Log.e("chengyake", "------------------ cmd beats");
        try {
            mDriver.write(beats, READ_WAIT_MILLIS);
        } catch (IOException e) {
            //Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
        }

        return 0;
    }*/

    public int writeAsync(String data){
        if(mReadThread.setCmd(data)==true)
            return 0;
        else
            return -1;
        /*
        //Log.e(TAG, "chengyake write Bytes true");
        mutex = true;
        uart_string = data;
        
        SystemClock.sleep(100);
        //Log.e("chengyake", "------------------ cmd string:" + data);
        data="\2" + data + "\3";
        try {
            mDriver.write(data.getBytes(), READ_WAIT_MILLIS);
        } catch (IOException e) {
            //Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
        }
*/


        /*
        try {
            mDriver.write(data.getBytes(), 200);
        } catch  (Exception e) {
            throw new IllegalStateException("upload error", e);
        }*/



    }

    public String detectCode(String data) {
        
        //Log.e("chengyake", "------------------ in detectCode");

        if(data.length()>150) {
            bigData++;
            if(bigData<7) {
                return CodeInfo.BEATS;
            } else if (bigData == 7) {
                return CodeInfo.INTO_PUS_C;
            } else {

                bigData=10;
                if(status==null || status.equals(""))
                status = UART_PUS;
                return null;
            }
        }
        if(data.equals(CodeInfo.BEATS)) {
            return CodeInfo.BEATS;
        } else if (data.equals(CodeInfo.UART_HANDSHAKE_1A)) {
            if(data.equals(mReadThread.getCmd())) {
                return CodeInfo.UART_HANDSHAKE_3C;
            }
            return CodeInfo.UART_HANDSHAKE_2C; 
        } else if (data.equals(CodeInfo.INTO_PUS_A )) {
                String ns = Context.NOTIFICATION_SERVICE;
                NotificationManager mNotificationManager = (NotificationManager) getSystemService(ns);
                int icon = R.drawable.icon;
                CharSequence tickerText = "通知栏标题";
                long when = System.currentTimeMillis();
                Notification notification = new Notification(icon, tickerText, when);
                Context context = getApplicationContext();
                CharSequence contentTitle = "展开标题";
                CharSequence contentText = "详细内容";
                notification.setLatestEventInfo(context, contentTitle, contentText,null); 
                mNotificationManager.notify(1, notification);

                return null;
        } else if (data.equals(CodeInfo.GET_INFO_1A)) {
            return CodeInfo.GET_INFO_2C;
        } else if (data.equals(CodeInfo.GET_INFO_2A)) {
            return CodeInfo.GET_INFO_3C; 
        } else if (data.equals(CodeInfo.GET_INFO_3A)) {
            return CodeInfo.GET_INFO_4C; 
        } else if (data.equals(CodeInfo.GET_INFO_4A)) {
            return CodeInfo.GET_INFO_5C; 
        } else if (data.equals(CodeInfo.EXIT_PUS_A)) {
            if(cpuType.equals(UART_PUS)) {
                status=UART_PUS;
                return CodeInfo.INTO_PUS_C; 
            }
            if(cpuType.equals(UART_PUM)) {
                status=UART_PUM;
                return CodeInfo.INTO_PUM_C; 
            }
        }
        //Log.e("chengyake", "------------------ in detectCode return null");

        return null;
    }

    public int uploadPUSConsole(String addr, String size) {


        StringBuffer addrBuf=new StringBuffer(); 
        StringBuffer sizeBuf=new StringBuffer();

        addrBuf.append(addr);
        sizeBuf.append(size);

        String mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
            addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
        String mSize = sizeBuf.substring(2, 4) + sizeBuf.substring(0, 2);

        uart_action = ACTION_UPLOAD_PUS_CONSOLE;
        return writeAsync(addZz(CodeInfo.UPLOAD_PUS_CONSOLE_C + mAddr + mSize + "0000"));
    }

        public int downloadPUSConsole(String addr, String data) {
            
            int i;

            if(addr == null || data == null){
                return -1;
            }

            StringBuffer addrBuf=new StringBuffer(); 
            StringBuffer dataBuf=new StringBuffer();


            int addrLen = addr.length();
            int dataLen = data.length();
                
            for(i=0; i<8-addrLen; i++) {
                addrBuf.append("0");
            }
            addrBuf.append(addr);

            for(i=0; i<4-dataLen; i++) {
                dataBuf.append("0");
            }

            dataBuf.append(data);
            
            String mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                           addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
            String mData = dataBuf.substring(2, 4) + dataBuf.substring(0, 2);

            uart_action = ACTION_DOWNLOAD_PUS_CONSOLE;
            return writeAsync(addZz("435357430C0001000100" + mAddr + "01000000" + mData + "0000"));
        }

        public int getPUSError() {
            uart_action = ACTION_GET_PUS_ERROR;
            return writeAsync(addZz(CodeInfo.GET_PUS_ERROR_C));
        }

        public int delPUSError() {
            uart_action = ACTION_DEL_PUS_ERROR;
            return writeAsync(addZz(CodeInfo.DEL_PUS_ERROR_C));
        }

        public int getPUMError() {
            uart_action = ACTION_GET_PUM_ERROR;
            return writeAsync(addZz(CodeInfo.GET_PUM_ERROR_C));
        }

        public int delPUMError() {
            uart_action = ACTION_DEL_PUM_ERROR;
            return writeAsync(addZz(CodeInfo.DEL_PUM_ERROR_C));
        }

        public int uploadPUSJobdata(String addr, String size) {
            
            int i;

            if(addr == null || size == null) {
                return -1;
            }

            StringBuffer addrBuf=new StringBuffer(); 
            StringBuffer sizeBuf=new StringBuffer();


            int addrLen = addr.length();
            int sizeLen = size.length();
                
            for(i=0; i<8-addrLen; i++) {
                addrBuf.append("0");
            }
            addrBuf.append(addr);

            for(i=0; i<4-sizeLen; i++) {
                sizeBuf.append("0");
            }

            sizeBuf.append(size);
            
            String mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                           addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
            String mSize = sizeBuf.substring(2, 4) + sizeBuf.substring(0, 2);

            //og.e("chengyake", "-------------" + mAddr + " ----- " + mSize);
            //og.e("chengyake", "------" + addZz(CodeInfo.UPLOAD_PUS_CONSOLE_C + mAddr + mSize + "0000"));

            uart_action = ACTION_UPLOAD_PUS_JOBDATA;
            return writeAsync(addZz("4353524A080001000100"+ mAddr + mSize + "0000"));
        }
        public int preDownloadPUSJobdata(String block) {
            //uart_action = ACTION_UPLOAD_PUS_JOBDATA;
            return writeAsync(addZz("435342420" + block.length()/2 + "0001000100" + block ));
        }

        public int downloadPUSJobdata(String addr, String size) {
            
            int i;

            if(addr == null || size == null){
                return -1;
            }

            StringBuffer addrBuf=new StringBuffer(); 
            StringBuffer sizeBuf=new StringBuffer();


            int addrLen = addr.length();
            int sizeLen = size.length();
                
            for(i=0; i<8-addrLen; i++) {
                addrBuf.append("0");
            }
            addrBuf.append(addr);

            for(i=0; i<2-sizeLen; i++) {
                sizeBuf.append("0");
            }

            sizeBuf.append(size);
            
            String mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                           addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
            String mSize = sizeBuf.substring(0, 2);

            //og.e("chengyake", "-------------" + mAddr + " ----- " + mSize);
            //og.e("chengyake", "------" + addZz(CodeInfo.UPLOAD_PUS_CONSOLE_C + mAddr + mSize + "0000"));

            uart_action = ACTION_DOWNLOAD_PUS_JOBDATA;
            return writeAsync(addZz( "4353574A050001000100" + mAddr + mSize));
        }

        public int flashPUSJobdata() {
            uart_action = ACTION_FLASH_PUS_JOBDATA;
            return writeAsync(CodeInfo.DOWNLOAD_PUS_JOBDATA_FLASH_C);
        }

        public int getPUSTime() {
            uart_action = ACTION_GET_PUS_TIME;
            return writeAsync(CodeInfo.GET_PUS_TIME_C);
        }
        public int setPUSTime(String time) {
            uart_action = ACTION_SET_PUS_TIME;
            return writeAsync(addZz("4353575A080001000100" + time + "00"));
        }

        public int uploadPUMConsole(String addr, String size) {

            StringBuffer addrBuf=new StringBuffer(); 
            StringBuffer sizeBuf=new StringBuffer();

            addrBuf.append(addr);
            sizeBuf.append(HexDump.toHexString(HexDump.hexStringToInt(size, 2)*2));
            //Log.e("chengyake", "---------------" + HexDump.toHexString(HexDump.hexStringToInt(size, 2)*2) + "------------------------");
            String mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
            String mSize = sizeBuf.substring(6, 8) + sizeBuf.substring(4, 6);

            uart_action = ACTION_UPLOAD_PUM_CONSOLE;
            return writeAsync(addZz(CodeInfo.UPLOAD_PUM_CONSOLE_C + mAddr + mSize + "0000"));
        }

        public int downloadPUMConsole(String addr, String data) {
            
            int i;

            if(addr == null || data == null){
                return -1;
            }

            StringBuffer addrBuf=new StringBuffer(); 
            StringBuffer dataBuf=new StringBuffer();


            int addrLen = addr.length();
            int dataLen = data.length();
                
            for(i=0; i<8-addrLen; i++) {
                addrBuf.append("0");
            }
            addrBuf.append(addr);

            for(i=0; i<4-dataLen; i++) {
                dataBuf.append("0");
            }

            dataBuf.append(data);
            
            String mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                           addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
            String mData = dataBuf.substring(2, 4) + dataBuf.substring(0, 2);

            uart_action = ACTION_DOWNLOAD_PUM_CONSOLE;
            return writeAsync(addZz("434D57430C0001000100" + mAddr + "02000000" + mData + "0000"));
        }
        public int uploadPUMJobdata(String addr, String size) {
            
            int i;

            if(addr == null || size == null) {
                return -1;
            }

            StringBuffer addrBuf=new StringBuffer(); 
            StringBuffer sizeBuf=new StringBuffer();


            int addrLen = addr.length();
            int sizeLen = size.length();
                
            for(i=0; i<8-addrLen; i++) {
                addrBuf.append("0");
            }
            addrBuf.append(addr);

            for(i=0; i<4-sizeLen; i++) {
                sizeBuf.append("0");
            }

            sizeBuf.append(HexDump.toHexString(HexDump.hexStringToInt(size, 2)*2));
            
            String mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                           addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
            String mSize = sizeBuf.substring(6, 8) + sizeBuf.substring(4, 6);

            //og.e("chengyake", "-------------" + mAddr + " ----- " + mSize);
            //og.e("chengyake", "------" + addZz(CodeInfo.UPLOAD_PUS_CONSOLE_C + mAddr + mSize + "0000"));

            uart_action = ACTION_UPLOAD_PUM_JOBDATA;
            return writeAsync(addZz("434D5243080001000100"+ mAddr + mSize + "0000"));
        }

        public int downloadPUMJobdata(String addr, String size) {
            
            int i;

            if(addr == null || size == null){
                return -1;
            }

            StringBuffer addrBuf=new StringBuffer(); 
            StringBuffer sizeBuf=new StringBuffer();


            int addrLen = addr.length();
            int sizeLen = size.length();
                
            for(i=0; i<8-addrLen; i++) {
                addrBuf.append("0");
            }
            addrBuf.append(addr);

            for(i=0; i<4-sizeLen; i++) {
                sizeBuf.append("0");
            }

            sizeBuf.append(size);
            
            String mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                           addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
            String mSize = sizeBuf.substring(2, 4) + sizeBuf.substring(0, 2);

            //og.e("chengyake", "-------------" + mAddr + " ----- " + mSize);
            //og.e("chengyake", "------" + addZz(CodeInfo.UPLOAD_PUS_CONSOLE_C + mAddr + mSize + "0000"));

            uart_action = ACTION_DOWNLOAD_PUM_JOBDATA;
            return writeAsync(addZz( "434D57430C0001000100" + mAddr + "02000000" + mSize + "0000"));
        }

        public int flashPUMJobdata() {
            uart_action = ACTION_FLASH_PUM_JOBDATA;
            return writeAsync(CodeInfo.DOWNLOAD_PUM_JOBDATA_FLASH_C);
        }

        public int getVersionInfo() {
            uart_action = ACTION_GET_INFO;
            return writeAsync(CodeInfo.GET_INFO_1C);
        }

        public String addZz(String str) {
            int i;
            byte[] bufs = HexDump.hexStringToByteArray(str);
            byte buf = 0;

            for(i=0; i<bufs.length; i++) {
                buf=(byte)(buf ^ bufs[i]);
            }

            return str + HexDump.toHexString(buf);
        }

        public int checkZz(String str) {

            return 0;

        }

        public String getUartStatus() {
            return  status;
        }
        public boolean getSyncStatus() {
            return  mutex;
        }

    public class MyBinder extends Binder{
        ComService getService()
        {
            return ComService.this;
        }
    }
    private MyBinder mBinder = new MyBinder();  


    static class IComServiceStub extends IComService.Stub {

        WeakReference<ComService> mService;
        IComServiceStub(ComService service) {
            mService = new WeakReference<ComService>(service);
        }
        public String getSystemTime() throws RemoteException {
            return mService.get().getSystemTime();
        }

        public int writeAsync(String data) {
            return mService.get().writeAsync(data);
        }


/**************************************************/


        public int probeCp2102() {
            return mService.get().probeCp2102();
        }
        public boolean getSyncStatus() {
            return mService.get().getSyncStatus();
        }
        public String getUartStatus() {
            return mService.get().getUartStatus();
        }
        public int getVersionInfo() {
            return mService.get().getVersionInfo();
        }
        public int heartBeats( ) {
            return mService.get().writeAsync(CodeInfo.BEATS);
        }


        //into exit cpu
        public int intoPUS( ) {
            return mService.get().writeAsync(CodeInfo.INTO_PUS_C);
        }
        public int exitPUS( ) {
            return mService.get().writeAsync(CodeInfo.EXIT_PUS_C);
        }
        public int intoPUM( ) {
            return mService.get().writeAsync(CodeInfo.INTO_PUM_C);
        }
        public int exitPUM( ) {
            return mService.get().writeAsync(CodeInfo.EXIT_PUM_C);
        }

        //pus console
        public int uploadPUSConsole(String addr, String size) {
            return mService.get().uploadPUSConsole(addr, size);
        }
        public int downloadPUSConsole(String addr, String data ) {
            return mService.get().downloadPUSConsole(addr, data);
        }

        //pus error data
        public int getPUSError() {
            return mService.get().getPUSError();
        }

        public int delPUSError() {
            return mService.get().delPUSError();
        }

        //pus jobdata
        public int uploadPUSJobdata(String addr, String size) {
            return mService.get().uploadPUSJobdata(addr, size);
        }
        public int preDownloadPUSJobdata(String block) {
            return mService.get().preDownloadPUSJobdata(block);
        }
        public int downloadPUSJobdata(String addr, String data) {
            return mService.get().downloadPUSJobdata(addr, data);
        }
        public int flashPUSJobdata() {
            return mService.get().flashPUSJobdata();
        }

        //pus sync time
        public int getPUSTime( ) {
            return mService.get().getPUSTime();
        }
        public int setPUSTime(String time) {
            return mService.get().setPUSTime(time);
        }
        //pum console
        public int uploadPUMConsole(String addr, String size) {
            return mService.get().uploadPUMConsole(addr, size);
        }
        public int downloadPUMConsole(String addr, String data ) {
            return mService.get().downloadPUMConsole(addr, data);
        }

        //pum error data
        public int getPUMError( ) {
            return mService.get().getPUMError();
        }
        public int delPUMError( ) {
            return mService.get().delPUMError();
        }

        //pum jobdata
        public int uploadPUMJobdata( String addr, String size) {
            return mService.get().uploadPUMJobdata(addr, size);
        }
        public int preDownloadPUMJobdata() {
            return mService.get().writeAsync(CodeInfo.DOWNLOAD_PUM_JOBDATA_PREPARE_C);
        }
        public int downloadPUMJobdata(String addr, String data) {
            return mService.get().downloadPUMJobdata(addr, data);
        }
        public int flashPUMJobdata() {
            return mService.get().flashPUMJobdata();
        }

    };
    private final IBinder aBinder = new IComServiceStub(this);
}



