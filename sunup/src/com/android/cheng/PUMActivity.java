package com.android.cheng;

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
import android.content.IntentFilter;  
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import android.content.Context;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.GridView;
import android.widget.ImageView;
import android.content.BroadcastReceiver;  
import java.util.ArrayList;
import java.util.HashMap;
import android.util.Log;

import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.SimpleAdapter; 
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;
import android.os.SystemClock;

import android.content.SharedPreferences;

import android.app.Notification;
import android.app.NotificationManager;



public class PUMActivity extends Activity {


	private TextView copyright = null;

    private IComService mIComService;

    Notification n;  
    NotificationManager nm;   

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver(){
        @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (action.equals(ComService.ACTION_FINISH)) {
                    Log.e("chengyake", "in PUMActivity finish");

                    nm.cancel(1);  
                    finish();
                }
            }

    };

    private ServiceConnection mServiceConnection = new ServiceConnection() {  
        public void onServiceConnected(ComponentName name, IBinder service) {  
            mIComService = IComService.Stub.asInterface(service);
        } 

        public void onServiceDisconnected(ComponentName name) {  
        }  
    }; 

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.pum_tab);
        GridView gridview = (GridView) findViewById(R.id.gridview);


		IntentFilter myIntentFilter = new IntentFilter();
		myIntentFilter.addAction(ComService.ACTION_FINISH);
		registerReceiver(mBroadcastReceiver, myIntentFilter);

        Intent j  = new Intent();  
        j.setClass(PUMActivity.this, ComService.class);
        getApplicationContext().bindService(j, mServiceConnection, BIND_AUTO_CREATE);

        ArrayList<HashMap<String, Object>> lstImageItem = new ArrayList<HashMap<String, Object>>();
        
        //console
        HashMap<String, Object> map1 = new HashMap<String, Object>();
        map1.put("ItemImage", R.drawable.console);
        map1.put("ItemText", "Console");
        lstImageItem.add(map1);

        //error data
        HashMap<String, Object> map2 = new HashMap<String, Object>();
        map2.put("ItemImage", R.drawable.errordata);
        map2.put("ItemText", "Error Data");
        lstImageItem.add(map2);

        //Job Data
        HashMap<String, Object> map3 = new HashMap<String, Object>();
        map3.put("ItemImage", R.drawable.jobdata);
        map3.put("ItemText", "JOB Data");
        lstImageItem.add(map3);


        SimpleAdapter saImageItems = new SimpleAdapter(this,  lstImageItem, R.layout.icon,
                new String[] {"ItemImage","ItemText"},  new int[] {R.id.ItemImage,R.id.ItemText});

        gridview.setAdapter(saImageItems);
        gridview.setOnItemClickListener(new ItemClickListener());

		copyright = (TextView) findViewById(R.id.copyright);
        
        //notify
        nm = (NotificationManager)this.getSystemService(NOTIFICATION_SERVICE);  

        n = new Notification();
        int icon = R.drawable.pum;
        String tickerText = "PUM";
        long when = System.currentTimeMillis();

        n.icon = icon;
        n.tickerText = tickerText;
        n.when = when;
        n.flags = Notification.FLAG_NO_CLEAR;  
        n.flags = Notification.FLAG_ONGOING_EVENT;   
        n.setLatestEventInfo(PUMActivity.this, "PUM", "PUM Status", null);  
        nm.notify(1, n);  


        Log.e("chengyake", "in PUMActivity onCreate func");
    }

    @Override  
    protected void onResume() {  
        super.onResume();
        Intent mIntent = new Intent();
        mIntent.setAction(ComService.ACTION_INTO_PUM);
        sendBroadcast(mIntent);
        nm.notify(1, n);  
        Log.e("chengyake", "onResume called.");
    }  

    @Override
    public void onBackPressed() {
    //public boolean onKeyDown(int keyCode, KeyEvent event) {
        Log.e("chengyake", "+++++++++++++++++++++");
	    new AlertDialog.Builder(PUMActivity.this).setTitle("确认退出吗？")
		.setIcon(android.R.drawable.ic_dialog_info)
		.setPositiveButton("确定", new DialogInterface.OnClickListener() {
		    @Override
		    public void onClick(DialogInterface dialog, int which) {
            Log.e("chengyake", "++++1+++++++++++++++++");
			    PUMActivity.this.finish();

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



    @Override  
    protected void onPause() {
        super.onPause();  
        Log.e("chengyake", "onPause called.");

        /*
        try {
            if(mIComService.getUartStatus() == ComService.UART_PUM) {
                if(mIComService.exitPUM() != 0) {
                    //busy wait...
                    //return;
                }
            }
        } catch  (Exception e) {
            throw new IllegalStateException("upload error", e);
        }*/


    }  

    @Override
    protected void onDestroy() {
          this.unregisterReceiver(mBroadcastReceiver);
          getApplicationContext().unbindService(mServiceConnection);

          nm.cancel(1);  
          super.onDestroy();
    }


    class  ItemClickListener implements OnItemClickListener
    {
        public void onItemClick(AdapterView<?> arg0,//The AdapterView where the click happened 
                View arg1,//The view within the AdapterView that was clicked
                int arg2,//The position of the view in the adapter
                long arg3//The row id of the item that was clicked
                ) {

            //HashMap<String, Object> item=(HashMap<String, Object>) arg0.getItemAtPosition(arg2);
            //setTitle((String)item.get("ItemText"));
            /*
            try {
                if(mIComService.getUartStatus() == ComService.UART_HANDSHAKED) {
                   if(mIComService.intoPUM() != 0) {
                       //busy wait...
                       return;
                   }
                }
            } catch  (Exception e) {
                throw new IllegalStateException("upload error", e);
            }*/



            SharedPreferences shareEdit = getSharedPreferences("sunupdata", 0); 
            if(shareEdit.getString("sunup_lisence", "false").equals("true") == false) {
                Toast.makeText(getApplicationContext(), "you need register by Verison Info",
                        Toast.LENGTH_SHORT).show();
                return;
            }



            Intent intent=new Intent();
            switch(arg2) {
                case 0:
                    intent.setClass(PUMActivity.this, PUMConsoleActivity.class);
                    startActivity(intent);
                    break;
                case 1:
                    intent.setClass(PUMActivity.this, PUMErrorDataActivity.class);
                    startActivity(intent);
                    break;
                case 2:
                    intent.setClass(PUMActivity.this, PUMJobDataActivity.class);
                    startActivity(intent);
                    break;

                default:
                    break;
            }

        }

    }
}


