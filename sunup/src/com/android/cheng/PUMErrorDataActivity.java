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
import android.view.View.OnFocusChangeListener;

import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;

import java.util.Date; 
import java.text.SimpleDateFormat;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.IntentFilter;  

import android.content.BroadcastReceiver;  
import java.util.HashMap;
import android.view.ContextMenu;
import android.view.MenuItem;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View.OnCreateContextMenuListener;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.AdapterView.OnItemClickListener;

import android.content.DialogInterface;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;

import android.widget.Toast;

import java.io.File;
import android.util.Log;
import jxl.Cell;
import jxl.CellReferenceHelper;
import jxl.CellType;
import jxl.Range;
import jxl.Sheet;
import jxl.Workbook;
import jxl.format.CellFormat;
import jxl.format.Colour;
import jxl.format.UnderlineStyle;
import jxl.read.biff.BiffException;
import jxl.write.Blank;
import jxl.write.DateFormat;
import jxl.write.DateFormats;
import jxl.write.DateTime;
import jxl.write.Formula;
import jxl.write.Label;
import jxl.write.Number;
import jxl.write.NumberFormat;
import jxl.write.WritableCell;
import jxl.write.WritableCellFeatures;
import jxl.write.WritableCellFormat;
import jxl.write.WritableFont;
import jxl.write.WritableHyperlink;
import jxl.write.WritableImage;
import jxl.write.WritableSheet;
import jxl.write.WritableWorkbook;
import jxl.write.WriteException;

import android.app.ProgressDialog;

public class PUMErrorDataActivity extends Activity {

    private ArrayList<HashMap<String, Object>> listItem;
    private SimpleAdapter listItemAdapter;
    private static int i=0;
    private ListView list;

    private IComService mIComService;

	private Button allerr;
	private Button errdel;
	private Button fileout;
    private ProgressDialog dialog; 
    private String file;

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
            if(action.equals(ComService.ACTION_GET_PUM_ERROR)){
                String info = intent.getExtras().getString("string"); 
                
                if(info.length() < 40) {
                    dialog.cancel();
                    return;
                }

                StringBuffer dataBuf=new StringBuffer(); 
                dataBuf.append(info);

                if(dataBuf.substring(48, 52).equals("0000")) {
                    //Toast.makeText(getApplicationContext(), "Get PUM Error Log Over and NO Error Item", Toast.LENGTH_SHORT).show();
                    return;
                }

                for(i=0; i<8; i++) {
                   HashMap<String, Object> map = new HashMap<String, Object>();
                   //map.put("etime", dataBuf.substring(i*64+20, i*64+34));
                   map.put("etime", dataBuf.substring(i*64+20, i*64+22) + "-" + dataBuf.substring(i*64+22, i*64+24) + "-" + dataBuf.substring(i*64+24, i*64+26) 
                          + " " + dataBuf.substring(i*64+28, i*64+30) + ":" + dataBuf.substring(i*64+30, i*64+32) + ":" + dataBuf.substring(i*64+32, i*64+34) );
                   map.put("ecode", dataBuf.substring(i*64+50, i*64+52) + dataBuf.substring(i*64+48, i*64+50));
                   map.put("estatus", dataBuf.substring(i*64+34, i*64+48));
                   map.put("esuppdata", dataBuf.substring(i*64+52, i*64+84));
                   listItem.add(0, map);
                }
                listItemAdapter.notifyDataSetChanged();

            } else if(action.equals(ComService.ACTION_DEL_PUM_ERROR)){
                dialog.cancel();
                Toast.makeText(getApplicationContext(), "Delete PUM ERROR Over", Toast.LENGTH_SHORT).show();
            } else if (action.equals(ComService.ACTION_FINISH)) {
                Log.e("chengyake", "in PUMConsoleActivity finish");
                finish();
            }
        }

    };


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.pum_error_data);

        allerr=(Button)findViewById(R.id.all_error);
        errdel=(Button)findViewById(R.id.del);
        fileout=(Button)findViewById(R.id.file_out);

		IntentFilter myIntentFilter = new IntentFilter();
		myIntentFilter.addAction(ComService.ACTION_GET_PUM_ERROR);
		myIntentFilter.addAction(ComService.ACTION_DEL_PUM_ERROR);
		myIntentFilter.addAction(ComService.ACTION_FINISH);
		registerReceiver(mBroadcastReceiver, myIntentFilter);


        allerr.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {

                dialog = new ProgressDialog(PUMErrorDataActivity.this); 
                dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER); 
                dialog.setTitle("PUM"); 
                dialog.setMessage("正在上传数据..."); 
                dialog.setIcon(android.R.drawable.ic_dialog_map); 
                dialog.setIndeterminate(false); 
                dialog.setCancelable(false);
                dialog.show(); 

                listItem.clear();
                listItemAdapter.notifyDataSetChanged();
                 try {
                     if(mIComService.getPUMError() != 0) {
                         return;
                     }
                 } catch  (Exception e) {
                     throw new IllegalStateException("getVersionInfo error", e);
                 }


			}
		});
        errdel.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {


            new AlertDialog.Builder(PUMErrorDataActivity.this)
            .setTitle("确定清除？")
            .setIcon(android.R.drawable.ic_dialog_info)
            .setPositiveButton("确定", new DialogInterface.OnClickListener() { 
                @Override  
                public void onClick(DialogInterface dia, int which) {  

                    dialog = new ProgressDialog(PUMErrorDataActivity.this); 
                    dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER); 
                    dialog.setTitle("PUM"); 
                    dialog.setMessage("正在清空数据..."); 
                    dialog.setIcon(android.R.drawable.ic_dialog_map); 
                    dialog.setIndeterminate(false); 
                    dialog.setCancelable(false);
                    dialog.show(); 

                     try {
                         if(mIComService.delPUMError() != 0) {
                             return;
                         }
                     } catch  (Exception e) {
                         throw new IllegalStateException("getVersionInfo error", e);
                     }
                }
            })
            .setNegativeButton("取消", null)
            .show();



			}
		});

        fileout.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
            
            SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmmss");
            String subName = df.format(new Date()); 
            file = "/sdcard/EPUM"+subName+".xls";

            new AlertDialog.Builder(PUMErrorDataActivity.this)
            .setTitle("确定导出到" + file + "？")
            .setIcon(android.R.drawable.ic_dialog_info)
            .setPositiveButton("确定", new DialogInterface.OnClickListener() { 
                @Override  
                public void onClick(DialogInterface dia, int which) {  
                    createExcel(file);
                }
            })
            .setNegativeButton("取消", null)
            .show();

			}
		});


        listItem = new ArrayList<HashMap<String, Object>>();
        list = (ListView) findViewById(R.id.pum_error_data_listview);

        listItemAdapter = new SimpleAdapter(this,listItem, 
                R.layout.pum_error_data_item,
                new String[] {"etime","ecode", "estatus", "esuppdata"}, 
                new int[] {R.id.etime,R.id.ecode,R.id.estatus, R.id.esuppdata}
                );

        list.setAdapter(listItemAdapter);

        list.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int arg2, long arg3) {
            HashMap<String, Object> map = listItem.get(arg2);
            if(map != null) {
                String eCode = (String) map.get("ecode");
                String Content = searchExcel("/sdcard/CONSOLE_ERRORCODE.xls", eCode);

                new AlertDialog.Builder(PUMErrorDataActivity.this)  
                .setTitle("错误代码：" + eCode)
                .setMessage(Content)
                .setPositiveButton("确定", null)
                .show();
            }

            }
        });

        list.setOnCreateContextMenuListener(new OnCreateContextMenuListener() {
            @Override
            public void onCreateContextMenu(ContextMenu menu, View v,ContextMenuInfo menuInfo) {
                //menu.setHeaderTitle("长按菜单-ContextMenu");   
                //menu.add(0, 0, 0, "弹出长按菜单0");
                //menu.add(0, 1, 0, "弹出长按菜单1");   
            }
        }); 

        Intent j  = new Intent();  
        j.setClass(this, ComService.class);
        getApplicationContext().bindService(j, mServiceConnection, BIND_AUTO_CREATE);

    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {

        //setTitle("点击了长按菜单里面的第"+item.getItemId()+"个项目"); 

        
        return super.onContextItemSelected(item);
    }


    @Override
    protected void onDestroy() {
        Log.e("chengyake", "Override in PUMErrorDataActivity onDestroy func");
        getApplicationContext().unbindService(mServiceConnection);
        this.unregisterReceiver(mBroadcastReceiver);
        super.onDestroy();
    }

    private static String searchExcel(String file, String eCode) {

        File inputWorkbook;
        try {
            inputWorkbook = new File(file);
            Workbook book = Workbook.getWorkbook(inputWorkbook);
            int num = book.getNumberOfSheets();  
            Sheet sheet = book.getSheet(0);
            int Rows = sheet.getRows();  
            for (int j = 1; j < Rows; ++j) {  
                if(eCode.equals(sheet.getCell(0, j).getContents())) {
                    return sheet.getCell(1, j).getContents();
                }
            }  
            Cell cell1 = sheet.getCell(0, 0);  
            String result = cell1.getContents();  
            System.out.println(result);  
            book.close();  
        } catch (Exception e) {  
            System.out.println(e);  
            return "文件/sdcard/CONSOLE_ERRORCODE.xls不存在";
        }  

        return "未知的错误代码";
    }

    private void createExcel(String path) {
        try {  

            WritableWorkbook book = Workbook.createWorkbook(new File(path));  
            WritableSheet sheet1 = book.createSheet("第一页", 0);  

            Label label;

            label = new Label(0, 0, "Time");  
            sheet1.addCell(label);  
            label = new Label(1, 0, "Code");  
            sheet1.addCell(label);  
            label = new Label(2, 0, "Status");  
            sheet1.addCell(label);  
            label = new Label(3, 0, "SuppData");
            sheet1.addCell(label);  


            for(int i=0; i<listItem.size(); i++) {
                Log.e("chengyake", "..........................." + i);
                HashMap<String, Object> map = listItem.get(i);
                if(map != null) {
                    String eTime = (String) map.get("etime");
                    String eCode = (String) map.get("ecode");
                    String eStatus = (String) map.get("estatus");
                    String eSuppdata = (String) map.get("esuppdata");
                    label = new Label(0, i+1, eTime);
                    sheet1.addCell(label);
                    label = new Label(1, i+1, eCode);
                    sheet1.addCell(label);
                    label = new Label(2, i+1, eStatus);
                    sheet1.addCell(label);
                    label = new Label(3, i+1, eSuppdata);
                    sheet1.addCell(label);
                } else {
                    break;
                }
            }

            book.write();
            book.close();

            Toast.makeText(getApplicationContext(), "文件" + path + "导出成功", Toast.LENGTH_SHORT).show();

        } catch (Exception e) {  
            System.out.println(e);  
            Toast.makeText(getApplicationContext(), "文件" + path + "导出失败", Toast.LENGTH_SHORT).show();
        }  
    }  


}



