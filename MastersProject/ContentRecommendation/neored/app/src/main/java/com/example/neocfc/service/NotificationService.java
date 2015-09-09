package com.example.neocfc.service;

import android.app.IntentService;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.Context;

import android.util.Log;

import com.example.neocfc.activity.RecommendationActivity;
import com.example.neocfc.neored.R;
import com.example.neocfc.util.Constants;
import com.example.neocfc.util.Util;

import java.io.File;

/**
 * An {@link IntentService} subclass for handling asynchronous task requests in
 * a service on a separate handler thread.
 * <p>
 * TODO: Customize class - update intent actions, extra parameters and static
 * helper methods.
 */
public class NotificationService extends IntentService {
    private static final String TAG = "NotificationService";
    private int notificationID = 23;
    public NotificationService() {
        super(NotificationService.class.getName());

    }
    public int onStartCommand(Intent intent, int flags, int startId) {

        return super.onStartCommand(intent,flags,startId);
    }
    @Override
    protected void onHandleIntent(Intent intent) {
        Log.v(TAG, "Show notifications");
        Intent pendingIntent = new Intent(this, RecommendationActivity.class);
        PendingIntent pIntent = PendingIntent.getActivity(this, (int) System.currentTimeMillis(), pendingIntent, 0);
        Notification.Builder mBuilder = new Notification.Builder(this)
                .setContentTitle("New recommendations from neored")
                .setContentText("Content")
                .setSmallIcon(R.mipmap.ic_launcher)
                .setContentIntent(pIntent)
                .setAutoCancel(true);


        NotificationManager notificationManager =
                (NotificationManager) getSystemService(NOTIFICATION_SERVICE);

        notificationManager.notify(notificationID, mBuilder.build());
    }
}
