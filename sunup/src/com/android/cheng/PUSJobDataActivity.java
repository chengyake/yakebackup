package com.android.cheng;

import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.os.IBinder;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.text.InputType;
import android.util.Log;
import java.io.IOException;
import android.graphics.Color;
import android.app.Activity;
import android.view.MenuItem;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnCreateContextMenuListener;
import android.view.View.OnFocusChangeListener;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.AdapterView.OnItemClickListener;
import android.content.Context;
import android.content.ServiceConnection;
import android.content.ComponentName;


import android.app.AlertDialog;
import android.app.AlertDialog.Builder;

import android.content.DialogInterface;
import com.android.cheng.util.HexDump;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.HashMap;
import android.content.BroadcastReceiver;  

import android.view.ViewGroup;
import android.content.Intent;
import android.content.IntentFilter;  

import android.widget.Toast;
import android.app.ProgressDialog;

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

import android.os.SystemClock;

public class PUSJobDataActivity extends Activity {

    private ArrayList<HashMap<String, Object>> listItem = new ArrayList<HashMap<String, Object>>();
    private MyAdapter listItemAdapter;
    private static int i=0;
    private ListView list;
    private IComService mIComService;

	private Button addfile;
	private Button upload;
	private Button download;
	private Button keyA;
	private Button keyB;
	private Button keyC;
	private Button keyD;
	private Button keyE;
	private Button keyF;
	private Button key0;
	private Button key1;
	private Button key2;
	private Button key3;
	private Button key4;
	private Button key5;
	private Button key6;
	private Button key7;
	private Button key8;
	private Button key9;
	private EditText addr;
	private EditText size;

    private String mAddr="";
    private String mSize="";
    private static int currentIF=-1;
    private static boolean keyBusy=false;
    private ProgressDialog dialog; 

    private static int download_error=0;

    private static int import_mode=0;

    private String newv;
    private String importCase;

    private AddFileThread mAddFileThread;

    class AddFileThread extends Thread {

        public void run() {
            String path="/sdcard/PUS500_DATA.xls";

            Intent intent1 = new Intent();
            intent1.setAction("PUSJobData_show_upload_dialog");
            sendBroadcast(intent1);
 
            listItem.clear();
            importCase = importExcel(path);

            import_mode=0;

            download_error = 0;
            Intent intent2 = new Intent();
            intent2.setAction("PUSJobData_cancel_download_dialog");
            sendBroadcast(intent2);

            Intent intent3 = new Intent();
            intent3.setAction("PUSJobData_show_import_case");
            sendBroadcast(intent3);
        }

    }
    class DownloadThread extends Thread {

        public void run() {

            Intent intent1 = new Intent();
            intent1.setAction("PUSJobData_show_download_dialog");
            sendBroadcast(intent1);

            downloadData();
        }

    }

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
            if(action.equals(ComService.ACTION_UPLOAD_PUS_JOBDATA)){
                String info = intent.getExtras().getString("string"); 
                
                int a,i;
                a = HexDump.hexStringToInt(mAddr, 4);
                listItemAdapter.setStartAddr(a);

                StringBuffer subBuf=new StringBuffer();
                StringBuffer newvBuf=new StringBuffer();
                subBuf.append(info);
                newvBuf.append(newv);

                //listItem.clear();
                //for(i=20; i<(2*HexDump.hexStringToInt(mSize, 2)+20); i+=2) {
                for(i=(2*HexDump.hexStringToInt(mSize, 2)+18); i>=20; i-=2) {
                   HashMap<String, Object> map = new HashMap<String, Object>();
                   map.put("ItemAddr", HexDump.toHexString(a+(i-20)/2));
                   map.put("ItemOld", subBuf.substring(i, 2+i));
                   if(import_mode == 1) {
                       map.put("ItemNew", newv.substring(10-(i-18), 10-(i-20)));
                       listItem.add(listItem.size(), map);
                   } else {
                       map.put("ItemNew", "");
                       listItem.add(0, map);
                   }
                }
                listItemAdapter.notifyDataSetChanged();
                if(import_mode!=1) {
                    dialog.cancel(); 
                }
            } else if(action.equals(ComService.ACTION_FLASH_PUS_JOBDATA)) {
                String info = intent.getExtras().getString("string"); 
                if(info.equals("2E414B000400010001004353424D3F")) {
                    dialog.cancel(); 
                }
            } else if (action.equals("PUSJobData_show_download_dialog")) {
                    dialog = new ProgressDialog(PUSJobDataActivity.this); 
                    dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER); 
                    dialog.setTitle("PUS"); 
                    dialog.setMessage("正在下载数据..."); 
                    dialog.setIcon(android.R.drawable.ic_dialog_map); 
                    dialog.setIndeterminate(false); 
                    dialog.setCancelable(false);
                    dialog.show();    
            } else if (action.equals("PUSJobData_cancel_download_dialog")) {
                    dialog.cancel();
                    if(download_error==1) {
                        Toast.makeText(getApplicationContext(), "addr or size is null; or device is busy", Toast.LENGTH_SHORT).show();
                    } else if(download_error==2) {
                        Toast.makeText(getApplicationContext(), "Address is protected",Toast.LENGTH_SHORT).show();
                    } else if(download_error == 3) {
                        Toast.makeText(getApplicationContext(), "没有数据需要下载", Toast.LENGTH_SHORT).show();
                    }
            } else if (action.equals("PUSJobData_show_upload_dialog")) {
                    dialog = new ProgressDialog(PUSJobDataActivity.this); 
                    dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER); 
                    dialog.setTitle("PUS"); 
                    dialog.setMessage("正在上传数据..."); 
                    dialog.setIcon(android.R.drawable.ic_dialog_map); 
                    dialog.setIndeterminate(false); 
                    dialog.setCancelable(false);
                    dialog.show();  
            } else if (action.equals("PUSJobData_show_import_case") && importCase!=null) {
                Toast.makeText(getApplicationContext(), importCase, Toast.LENGTH_SHORT).show();
            } else if (action.equals(ComService.ACTION_FINISH)) {
                Log.e("chengyake", "in PUSJobDataActivity finish");
                finish();
            }
        }

    };

    private class MyAdapter extends SimpleAdapter {
        int currentPos = -1;
        int startPos = -1;
        int count = 0;
        private ArrayList<HashMap<String, Object>> mItemList;
        public MyAdapter(Context context, ArrayList<HashMap<String, Object>> data,
                int resource, String[] from, int[] to) {
            super(context, data, resource, from, to);
            mItemList = (ArrayList<HashMap<String, Object>>) data;
            if(data == null) {
                count = 0;
            } else {
                count = data.size();
            }
        }

        public void setCurrent(int pos) {
            currentPos = pos;
        }
        public void setStartAddr(int start) {
            startPos = start;
        }
        public int getCount() {
            return mItemList.size();
        }

        public Object getItem(int pos) {
            return pos;
        }

        public long getItemId(int pos) {
            return pos;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View view = super.getView(position, convertView, parent);
            TextView mNew = (TextView)view.findViewById(R.id.ItemNew);
            TextView mOld = (TextView)view.findViewById(R.id.ItemOld);
            TextView mAdd = (TextView)view.findViewById(R.id.ItemAddr);

            mNew.setTextColor(Color.GRAY);
            mOld.setTextColor(Color.GRAY);
            mAdd.setTextColor(Color.GRAY);

            if((!mNew.getText().toString().equals("")) && 
                    (!mNew.getText().toString().equals(mOld.getText().toString()))) {

                mNew.setTextColor(Color.RED);
                mOld.setTextColor(Color.RED);
                mAdd.setTextColor(Color.RED);

            }
           if(currentPos >= 0 && currentPos < mItemList.size())  {
                HashMap<String, Object> map = mItemList.get(currentPos);

                if(map != null) {
                    String eAddr = (String) map.get("ItemAddr");
                    if(mAdd.getText().toString().equals(eAddr))  {
                        mNew.setTextColor(Color.GREEN);
                        mOld.setTextColor(Color.GREEN);
                        mAdd.setTextColor(Color.GREEN);
                    }
                }
            }
            return view;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.pus_job_data);

		IntentFilter myIntentFilter = new IntentFilter();
		myIntentFilter.addAction(ComService.ACTION_UPLOAD_PUS_JOBDATA);
		myIntentFilter.addAction(ComService.ACTION_FLASH_PUS_JOBDATA);
		myIntentFilter.addAction("PUSJobData_show_download_dialog");
		myIntentFilter.addAction("PUSJobData_cancel_download_dialog");
		myIntentFilter.addAction("PUSJobData_show_import_case");
		myIntentFilter.addAction("PUSJobData_show_upload_dialog");
		myIntentFilter.addAction(ComService.ACTION_FINISH);
		registerReceiver(mBroadcastReceiver, myIntentFilter);


        addfile=(Button)findViewById(R.id.add_file);
        list = (ListView) findViewById(R.id.pus_job_data_listview);
        upload=(Button)findViewById(R.id.pus_job_data_upload);
        download=(Button)findViewById(R.id.pus_job_data_download);
        keyA=(Button)findViewById(R.id.pus_job_data_keyA);
        keyB=(Button)findViewById(R.id.pus_job_data_keyB);
        keyC=(Button)findViewById(R.id.pus_job_data_keyC);
        keyD=(Button)findViewById(R.id.pus_job_data_keyD);
        keyE=(Button)findViewById(R.id.pus_job_data_keyE);
        keyF=(Button)findViewById(R.id.pus_job_data_keyF);
        key0=(Button)findViewById(R.id.pus_job_data_key0);
        key1=(Button)findViewById(R.id.pus_job_data_key1);
        key2=(Button)findViewById(R.id.pus_job_data_key2);
        key3=(Button)findViewById(R.id.pus_job_data_key3);
        key4=(Button)findViewById(R.id.pus_job_data_key4);
        key5=(Button)findViewById(R.id.pus_job_data_key5);
        key6=(Button)findViewById(R.id.pus_job_data_key6);
        key7=(Button)findViewById(R.id.pus_job_data_key7);
        key8=(Button)findViewById(R.id.pus_job_data_key8);
        key9=(Button)findViewById(R.id.pus_job_data_key9);

        addr=(EditText)findViewById(R.id.pus_job_data_addr);
        size=(EditText)findViewById(R.id.pus_job_data_size);

        addr.setInputType(InputType.TYPE_NULL);
        size.setInputType(InputType.TYPE_NULL);

        addfile.setOnClickListener(new View.OnClickListener() {
            @Override 
			public void onClick(View v) { 
			    import_mode=1;

                

                new AlertDialog.Builder(PUSJobDataActivity.this)
                .setTitle("确定导入PUS文件？")
                .setIcon(android.R.drawable.ic_dialog_info)
                .setPositiveButton("确定", new DialogInterface.OnClickListener() { 
                    @Override  
                    public void onClick(DialogInterface dia, int which) {  
                        mAddFileThread = new AddFileThread();
                        mAddFileThread.start();
                    }
                        
                })
                .setNegativeButton("取消", null)
                .show();






             
			}
		});

        addr.setOnFocusChangeListener(new View.OnFocusChangeListener() { 
           @Override 
           public void onFocusChange(View v, boolean hasFocus) { 
                if(hasFocus){ 
                    addr.setBackgroundColor(Color.GREEN);
                    currentIF=-1;
                    listItemAdapter.setCurrent(currentIF);
                    listItemAdapter.notifyDataSetChanged();
                } else {
                    addr.setBackgroundColor(Color.WHITE);
                } 
           } 
        });

        size.setOnFocusChangeListener(new View.OnFocusChangeListener() { 
           @Override 
           public void onFocusChange(View v, boolean hasFocus) { 
                if(hasFocus){ 
                    size.setBackgroundColor(Color.GREEN);
                    currentIF=-1;
                    listItemAdapter.setCurrent(currentIF);
                    listItemAdapter.notifyDataSetChanged();
                } else {
                    size.setBackgroundColor(Color.WHITE);
                } 
           } 
        });

        upload.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 

                listItem.clear();

                dialog = new ProgressDialog(PUSJobDataActivity.this); 
                dialog.setProgressStyle(ProgressDialog.STYLE_SPINNER); 
                dialog.setTitle("PUS"); 
                dialog.setMessage("正在上传数据..."); 
                dialog.setIcon(android.R.drawable.ic_dialog_map); 
                dialog.setIndeterminate(false); 
                dialog.setCancelable(false);
                dialog.show();    

                upload();

			}
		});

        download.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {

                new AlertDialog.Builder(PUSJobDataActivity.this)
                .setTitle("确定下载？")
                .setIcon(android.R.drawable.ic_dialog_info)
                .setPositiveButton("确定", new DialogInterface.OnClickListener() { 
                    @Override  
                    public void onClick(DialogInterface dia, int which) {  

                        DownloadThread mDownloadThread = new DownloadThread();
                        mDownloadThread.start();
                    }
                        
                })
                .setNegativeButton("取消", null)
                .show();
            }
		});

		//key 0 - key 9 key A- key F
        keyA.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "A";
                       addr.setText(mAddr);
                   } else if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "A";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "A");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }
               } catch  (Exception e) {
                   throw new IllegalStateException("keyA error", e);
               }
			}
		});

        keyB.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "B";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "B";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "B");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("keyB error", e);
               }
			}
		});

        keyC.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "C";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "C";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "C");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("keyC error", e);
               }
			}
		});

        keyD.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "D";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "D";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "D");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("keyA error", e);
               }
			}
		});

        keyE.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "E";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "E";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "E");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("keyA error", e);
               }
			}
		});

        keyF.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "F";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "F";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "F");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("keyA error", e);
               }
			}
		});

        key0.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "0";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "0";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "0");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key0 error", e);
               }
			}
		});

        key0.setOnLongClickListener(new View.OnLongClickListener() {
            @Override
			public boolean onLongClick(View v) { 
               try {
                   if(addr.hasFocus()) {
                       mAddr="";
                       addr.setText("0");
                   }
                   if(size.hasFocus()) {
                       mSize="";
                       size.setText("0");
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       map.put("ItemNew", "");
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                       }
               } catch  (Exception e) {
                   throw new IllegalStateException("key0 error", e);
               }

               return true;
			}
		});

        key1.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "1";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "1";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "1");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key1 error", e);
               }
			}
		});

        key2.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "2";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "2";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "2");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key2 error", e);
               }
			}
		});

        key3.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "3";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "3";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "3");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key3 error", e);
               }
			}
		});
        key4.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "4";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "4";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "4");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key4 error", e);
               }
			}
		});

        key5.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "5";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "5";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "5");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key5 error", e);
               }
			}
		});

        key6.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "6";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "6";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "6");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key6 error", e);
               }
			}
		});

        key7.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "7";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "7";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "7");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key7 error", e);
               }
			}
		});

        key8.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "8";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "8";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "8");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key8 error", e);
               }
			}
		});

        key9.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) { 
               try {
                   if(addr.hasFocus() && mAddr.length()<8 ) {
                       mAddr=mAddr + "9";
                       addr.setText(mAddr);
                   }
                   if(size.hasFocus() && mSize.length()<4) {
                       mSize=mSize + "9";
                       size.setText(mSize);
                   } else if(currentIF>=0) {
                       HashMap<String, Object> map = listItem.remove(currentIF);
                       String check = (String) map.get("ItemNew");
                       if(check.length()<2) {
                            map.put("ItemNew", check + "9");
                       }
                       listItem.add(currentIF, map);
                       listItemAdapter.notifyDataSetChanged();
                   }

               } catch  (Exception e) {
                   throw new IllegalStateException("key9 error", e);
               }
			}
		});


        listItemAdapter = new MyAdapter(this,listItem, 
                R.layout.pus_job_data_item,
                new String[] {"ItemAddr","ItemOld", "ItemNew"}, 
                new int[] {R.id.ItemAddr,R.id.ItemOld,R.id.ItemNew}
                );

        list.setAdapter(listItemAdapter);

        list.setOnItemClickListener(new OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int arg2, long arg3) {
            //setTitle("点击第"+arg2+"个项目");
            addr.clearFocus();
            size.clearFocus();
            currentIF=arg2;
            listItemAdapter.setCurrent(currentIF);
            listItemAdapter.notifyDataSetChanged();
            //listItemAdapter.setCurrent(0);
            }

        });

        list.setOnCreateContextMenuListener(new OnCreateContextMenuListener() {
            @Override
            public void onCreateContextMenu(ContextMenu menu, View v,ContextMenuInfo menuInfo) {
                //menu.setHeaderTitle("长按菜单-ContextMenu");   
                /*
                HashMap<String, Object> map = new HashMap<String, Object>();
                map.put("ItemAddr", "add");
                map.put("ItemOld", "3");
                map.put("ItemNew", "");
                listItem.add(map);
                listItemAdapter.mItemList = listItem;
                */
                listItemAdapter.notifyDataSetChanged();
            }
        }); 



        Intent j  = new Intent();  
        j.setClass(PUSJobDataActivity.this, ComService.class);
        getApplicationContext().bindService(j, mServiceConnection, BIND_AUTO_CREATE);

        Log.e("chengyake", "in PUSJobDataActivity onCreate");
    }

    @Override
    public boolean onContextItemSelected(MenuItem item) {
        //setTitle("长按"+item.getItemId()); 

        
        return super.onContextItemSelected(item);
    }

    @Override
    protected void onDestroy() {
        Log.e("chengyake", "Override in PUSJobDataActivity onDestroy func");
        getApplicationContext().unbindService(mServiceConnection);
        this.unregisterReceiver(mBroadcastReceiver);
        super.onDestroy();
    }
    private void downloadData() {

            if(keyBusy == true || mAddr == null || mSize == null || mAddr.equals("") || mSize.equals("")) {
                download_error=1;
                Intent intent1 = new Intent();
                intent1.setAction("PUSJobData_cancel_download_dialog");
                sendBroadcast(intent1);
                return ;
            }

            if(HexDump.hexStringToInt(mAddr, 4) < 0x600000 || HexDump.hexStringToInt(mAddr, 4) > 0x66ffff) {
                download_error=2;
                Intent intent1 = new Intent();
                intent1.setAction("PUSJobData_cancel_download_dialog");
                sendBroadcast(intent1);
                return;
            }

            keyBusy = true;
            //pre download
            //write into mem
            String block="";
            for(int j=0; j<listItemAdapter.getCount(); j++) {
                HashMap<String, Object> map = listItem.get(j);
                String checkNew = (String) map.get("ItemNew");
                String checkOld = (String) map.get("ItemOld");
                if(!checkNew.equals("") && !checkOld.equals(checkNew)) {
                    StringBuffer Buf=new StringBuffer(); 
                    Buf.append((String) map.get("ItemAddr"));
                    if(Buf.substring(2,4).equals("60") && !block.contains("67")) {
                        block=block+"67";
                    } else if(Buf.substring(2,4).equals("61") && !block.contains("68")) {
                        block=block+"68";
                    } else if(Buf.substring(2,4).equals("62") && !block.contains("69")) {
                        block=block+"69";
                    } else if(Buf.substring(2,4).equals("63") && !block.contains("6A")) {
                        block=block+"6A";
                    } else if(Buf.substring(2,4).equals("64") && !block.contains("6B")) {
                        block=block+"6B";
                    } else if(Buf.substring(2,4).equals("65") && !block.contains("6C")) {
                        block=block+"6C";
                    } else if(Buf.substring(2,4).equals("66") && !block.contains("6D")) {
                        block=block+"6D";
                    } 
                }
			}
			if(block.equals("")) {
                keyBusy=false;
                download_error=3;
                Intent intent1 = new Intent();
                intent1.setAction("PUSJobData_cancel_download_dialog");
                sendBroadcast(intent1);
                return;
            }
            try {
                 while(-1 == mIComService.preDownloadPUSJobdata(block));
            } catch  (Exception e) {
                 throw new IllegalStateException("upload error", e);
            }

            //write into mem
            for(int j=0; j<listItemAdapter.getCount(); j++) {
                HashMap<String, Object> map = listItem.remove(j);
                String checkNew = (String) map.get("ItemNew");
                String checkOld = (String) map.get("ItemOld");
                if(!checkNew.equals("") && !checkOld.equals(checkNew)) {
                    try {
                         while(-1 == mIComService.downloadPUSJobdata((String)map.get("ItemAddr"), checkNew));
                    } catch  (Exception e) {
                         throw new IllegalStateException("upload error", e);
                    }
                }
                listItem.add(j, map);
                //listItemAdapter.notifyDataSetChanged();
			}

            //flash into flash
            try {
                 while(-1 == mIComService.flashPUSJobdata());
            } catch  (Exception e) {
                 throw new IllegalStateException("upload error", e);
            }

            keyBusy = false;

    }

    private void upload() {

                int i;
                if(keyBusy == true || mAddr == null || mSize == null || mAddr.equals("") || mSize.equals("")){
                    dialog.cancel(); 
                    Toast.makeText(getApplicationContext(), "addr or size is null; or device is busy", Toast.LENGTH_SHORT).show();
                    return ;
                }

                keyBusy = true;
                StringBuffer addrBuf=new StringBuffer(); 
                StringBuffer sizeBuf=new StringBuffer();

                int addrLen = mAddr.length();
                int sizeLen = mSize.length();
                    
                for(i=0; i<8-addrLen; i++) {
                    addrBuf.append("0");
                }
                addrBuf.append(mAddr);

                for(i=0; i<4-sizeLen; i++) {
                    sizeBuf.append("0");
                }
                sizeBuf.append(mSize);

                /*
                mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                               addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
                mSize = sizeBuf.substring(2, 4) + sizeBuf.substring(0, 2);
                */

                mAddr = addrBuf.substring(0, 8);
                mSize = sizeBuf.substring(0, 4);

               try {
                   mIComService.uploadPUSJobdata(mAddr, mSize);
                   //list.setSelection(14);
                   //if(size.hasFocus()) {
               } catch  (Exception e) {
                   throw new IllegalStateException("upload error", e);
               }
               keyBusy = false;

    }

    private String importExcel(String file) {

        File inputWorkbook;
        try {
            inputWorkbook = new File(file);
            Workbook book = Workbook.getWorkbook(inputWorkbook);
            int num = book.getNumberOfSheets();  
            Sheet sheet = book.getSheet(0);
            int Rows = sheet.getRows();  

            for (int j = 0; j < Rows; j++) {
                String addr = sheet.getCell(0, j).getContents();
                newv = sheet.getCell(1, j).getContents();
                String size = sheet.getCell(2, j).getContents();
                if(addr.contains(";") || addr==null || addr.equals("")) {
                    continue;
                }

                Log.e("chengyake", "......1......." + addr + " "  +  newv + " " + size);
                //fen ge
                StringBuffer addrBuf=new StringBuffer(); 
                addrBuf.append(addr);
                mAddr = addrBuf.substring(2, 10);
                mSize = size;

                do{
                    SystemClock.sleep(10);
                } while(0!=uploadInv());
                    SystemClock.sleep(1000);
            }
            book.close();  
        } catch (Exception e) {
            System.out.println(e);  
            return "文件/sdcard/PUS500_DATA.xls不存在";
        }

        return "文件/sdcard/PUS500_DATA.xls导入完成";
    }

    private int uploadInv() {

                int i;
                if(keyBusy == true || mAddr == null || mSize == null || mAddr.equals("") || mSize.equals("")){
                    dialog.cancel(); 
                    Toast.makeText(getApplicationContext(), "addr or size is null; or device is busy", Toast.LENGTH_SHORT).show();
                    return -1;
                }

                keyBusy = true;
                StringBuffer addrBuf=new StringBuffer(); 
                StringBuffer sizeBuf=new StringBuffer();

                int addrLen = mAddr.length();
                int sizeLen = mSize.length();
                    
                for(i=0; i<8-addrLen; i++) {
                    addrBuf.append("0");
                }
                addrBuf.append(mAddr);

                for(i=0; i<4-sizeLen; i++) {
                    sizeBuf.append("0");
                }
                sizeBuf.append(mSize);

                /*
                mAddr = addrBuf.substring(6, 8) + addrBuf.substring(4, 6) +
                               addrBuf.substring(2, 4) + addrBuf.substring(0, 2);
                mSize = sizeBuf.substring(2, 4) + sizeBuf.substring(0, 2);
                */

                mAddr = addrBuf.substring(0, 8);
                mSize = sizeBuf.substring(0, 4);

               try {
                   mIComService.uploadPUSJobdata(mAddr, mSize);
                   //list.setSelection(14);
                   //if(size.hasFocus()) {
               } catch  (Exception e) {
                   throw new IllegalStateException("upload error", e);
               }
               keyBusy = false;

               return 0;


    }

}

