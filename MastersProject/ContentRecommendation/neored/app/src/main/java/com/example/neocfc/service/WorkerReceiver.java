package com.example.neocfc.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.example.neocfc.util.Util;

public class WorkerReceiver extends BroadcastReceiver {
    private  String TAG = "WorkerReceiver";
    public WorkerReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.v(TAG, "WorkerReceiver Alarm triggered");
        boolean service_Running = Util.isMyServiceRunning(context, WorkerService.class);
        if(service_Running == false){
            Util.startWorkerService(context);
        }
        else{
            Log.v(TAG,"WorkerReceiver Service already running");
        }
    }
}
