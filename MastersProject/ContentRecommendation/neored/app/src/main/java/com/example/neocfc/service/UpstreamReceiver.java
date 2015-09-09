package com.example.neocfc.service;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import com.example.neocfc.util.Util;

public class UpstreamReceiver extends BroadcastReceiver {

    private  String TAG = "UpstreamReceiver";
    public UpstreamReceiver() {
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO: This method is called when the BroadcastReceiver is receiving
        // an Intent broadcast.
        Log.v(TAG, "Upstream Alarm triggered");
        boolean upservice_Running = Util.isMyServiceRunning(context,Upstream.class);
        boolean feedbackservice_Running = Util.isMyServiceRunning(context,FeedbackService.class);
        if(upservice_Running == false){
            Util.startUpstreamService(context);
        }
        else{
            Log.v(TAG,"Upstream Service already running");
        }
        if(feedbackservice_Running == false){
            Util.startFeedbackService(context);
        }
        else{
            Log.v(TAG,"Feedback Service already running");
        }
    }
}
