package com.example.neocfc.com.example.neocfc.network;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;

import com.example.neocfc.util.Util;

public class ConnectivityReceiver extends BroadcastReceiver {
    public ConnectivityReceiver() {
    }
    public String TAG = "ConnectivityReceiver";


    private boolean isOnline(Context mContext)  {
        ConnectivityManager cm = (ConnectivityManager)mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo ni = cm.getActiveNetworkInfo();
        return (ni != null && ni.isConnected());
    }

    @Override
    public void onReceive(Context context, Intent intent) {


        if(isOnline(context)==false){
                Log.v(TAG, "Connectivity Receiver network not connected");
            }else{
                Log.v(TAG, "Connectivity Receiver network  connected.Starting up/down alarms");
                Util.startBackgroundDaemons(context);
            }

        }

}
