package com.android.cheng;


interface IComService {
    int probeCp2102(); 
    String getSystemTime();
    int writeAsync(String data );
    String getUartStatus();
    boolean getSyncStatus(); 
    int getVersionInfo(); 
    int heartBeats(); 
    int intoPUS(); 
    int exitPUS(); 
    int intoPUM(); 
    int exitPUM(); 
    int uploadPUSConsole(String addr, String size); 
    int downloadPUSConsole(String addr, String data); 
    int getPUSError(); 
    int delPUSError(); 
    int uploadPUSJobdata(String addr, String size); 
    int preDownloadPUSJobdata(String block); 
    int downloadPUSJobdata(String addr, String data); 
    int flashPUSJobdata(); 
    int getPUSTime(); 
    int setPUSTime(String time); 
    int uploadPUMConsole(String addr, String size); 
    int downloadPUMConsole(String addr, String data); 
    int getPUMError(); 
    int delPUMError(); 
    int uploadPUMJobdata(String addr, String size); 
    int preDownloadPUMJobdata(); 
    int downloadPUMJobdata(String addr, String data); 
    int flashPUMJobdata(); 

}
