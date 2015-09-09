package com.example.neocfc.com.example.neocfc.network;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;

import com.example.neocfc.util.Util;

public class BootReceiver extends BroadcastReceiver {
    public BootReceiver() {
    }
    public String TAG = "BootReceiver";
    @Override
    public void onReceive(Context context, Intent intent) {
            Log.v(TAG,"Application restarted on boot");
            Util.startBackgroundDaemons(context);
        }
}
