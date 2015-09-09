package com.example.neocfc.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.example.neocfc.util.Util;

public class NotificationReceiver extends BroadcastReceiver {
    private  String TAG = "NotificationReceiver";
    public NotificationReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.v(TAG, "NotificationReceiver Alarm triggered");
        boolean service_Running = Util.isMyServiceRunning(context, NotificationService.class);
        if(service_Running == false){
            Util.startNotificationService(context);
        }
        else{
            Log.v(TAG,"NotificationReceiver Service already running");
        }
    }
}