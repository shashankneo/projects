package com.example.neocfc.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.example.neocfc.util.Util;

public class DownstreamReceiver extends BroadcastReceiver {

    private  String TAG = "DownstreamReceiver";
    public DownstreamReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO: This method is called when the BroadcastReceiver is receiving
        // an Intent broadcast.

        Log.v(TAG, "Downstream Alarm triggered");
        boolean service_Running = Util.isMyServiceRunning(context, DownStream.class);
        if(service_Running == false){
            Util.startDownstreamService(context);
        }
        else{
            Log.v(TAG,"DownStream Service already running");
        }
    }
}
