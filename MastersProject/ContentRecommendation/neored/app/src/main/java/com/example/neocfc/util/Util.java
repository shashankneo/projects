package com.example.neocfc.util;

import android.app.ActivityManager;
import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Environment;
import android.os.SystemClock;
import android.util.Log;

import com.example.neocfc.activity.MainActivity;
import com.example.neocfc.activity.RecommendationActivity;
import com.example.neocfc.service.DownStream;
import com.example.neocfc.service.DownstreamReceiver;
import com.example.neocfc.service.FeedbackService;
import com.example.neocfc.service.NotificationReceiver;
import com.example.neocfc.service.NotificationService;
import com.example.neocfc.service.Upstream;
import com.example.neocfc.service.UpstreamReceiver;
import com.example.neocfc.service.WorkerReceiver;
import com.example.neocfc.service.WorkerService;
import com.facebook.AccessToken;
import com.facebook.FacebookSdk;
import com.facebook.LoggingBehavior;
import com.twitter.sdk.android.Twitter;
import com.twitter.sdk.android.core.TwitterAuthConfig;
import com.twitter.sdk.android.core.TwitterAuthToken;
import com.twitter.sdk.android.core.TwitterSession;

import org.json.JSONArray;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;

import io.fabric.sdk.android.Fabric;

/**
 * Created by neocfc on 6/30/15.
 */
public class Util {
    static public final String TAG = "Util";
    static public void startBackgroundDaemons(Context mContext){
        Util.startWorkerAlarm(mContext);
        Util.startUpAlarm(mContext);
        Util.startDownAlarm(mContext);
        Util.startNotificationAlarm(mContext);
    }
    static public void startDownAlarm(Context mContext) {
        Intent alarmIntent = new Intent(mContext, DownstreamReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, alarmIntent, PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager alarmManager = (AlarmManager) mContext
                .getSystemService(Context.ALARM_SERVICE);

        alarmManager.setRepeating(AlarmManager.ELAPSED_REALTIME,
                SystemClock.elapsedRealtime(),
                AlarmManager.INTERVAL_HOUR, pendingIntent);
    }

    static public void cancelDownAlarm(Context mContext) {
        AlarmManager alarmManager = (AlarmManager) mContext
                .getSystemService(Context.ALARM_SERVICE);

        Intent alarmIntent = new Intent(mContext, DownstreamReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, alarmIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        alarmManager.cancel(pendingIntent);
    }

    static public void startUpAlarm(Context mContext) {
        Intent alarmIntent = new Intent(mContext, UpstreamReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, alarmIntent, PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager alarmManager = (AlarmManager) mContext
                .getSystemService(Context.ALARM_SERVICE);

        alarmManager.setRepeating(AlarmManager.ELAPSED_REALTIME,
                SystemClock.elapsedRealtime(),
                AlarmManager.INTERVAL_HOUR, pendingIntent);
    }

    static public void cancelUpAlarm(Context mContext) {
        AlarmManager alarmManager = (AlarmManager) mContext
                .getSystemService(Context.ALARM_SERVICE);

        Intent alarmIntent = new Intent(mContext, UpstreamReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, alarmIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        alarmManager.cancel(pendingIntent);
    }

    static public void startWorkerAlarm(Context mContext) {
        Intent alarmIntent = new Intent(mContext, WorkerReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, alarmIntent, PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager alarmManager = (AlarmManager) mContext
                .getSystemService(Context.ALARM_SERVICE);

        alarmManager.setRepeating(AlarmManager.ELAPSED_REALTIME,
                SystemClock.elapsedRealtime(),
                AlarmManager.INTERVAL_HOUR, pendingIntent);
    }

    static public void cancelWorkerAlarm(Context mContext) {
        AlarmManager alarmManager = (AlarmManager) mContext
                .getSystemService(Context.ALARM_SERVICE);

        Intent alarmIntent = new Intent(mContext, WorkerReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, alarmIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        alarmManager.cancel(pendingIntent);
    }

    static public void startNotificationAlarm(Context mContext) {
        Intent alarmIntent = new Intent(mContext, NotificationReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, alarmIntent, PendingIntent.FLAG_CANCEL_CURRENT);
        AlarmManager alarmManager = (AlarmManager) mContext
                .getSystemService(Context.ALARM_SERVICE);

        alarmManager.setRepeating(AlarmManager.ELAPSED_REALTIME,
                SystemClock.elapsedRealtime(),
                AlarmManager.INTERVAL_HOUR, pendingIntent);
    }

    static public void cancelNotificationAlarm(Context mContext) {
        AlarmManager alarmManager = (AlarmManager) mContext
                .getSystemService(Context.ALARM_SERVICE);

        Intent alarmIntent = new Intent(mContext, NotificationReceiver.class);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, alarmIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        alarmManager.cancel(pendingIntent);
    }

    static public void startUpKit(Context mContext){
        final String TWITTER_KEY = "i40fO7mMhRHFed8uxd588lDFL";
        final String TWITTER_SECRET = "sxQPYSqRTHTEmCf79ImwVXkD7KjL2AWx7csjkwmD40pDVLFGYv";
        FacebookSdk.sdkInitialize(mContext.getApplicationContext());
        FacebookSdk.addLoggingBehavior(LoggingBehavior.REQUESTS);
        TwitterAuthConfig authConfig = new TwitterAuthConfig(TWITTER_KEY, TWITTER_SECRET);
        Fabric.with(mContext, new Twitter(authConfig));
    }
    static public void startWorkerService(Context mContext) {
        Intent intent = new Intent(mContext, WorkerService.class);
        mContext.startService(intent);
    }
    static public void startNotificationService(Context mContext) {
        Intent intent = new Intent(mContext, NotificationService.class);
        mContext.startService(intent);
    }
    static public void startUpstreamService(Context mContext) {
        Intent intent = new Intent(mContext, Upstream.class);
        mContext.startService(intent);
    }
    static public void startFeedbackService(Context mContext) {
        Intent intent = new Intent(mContext, FeedbackService.class);
        mContext.startService(intent);
    }
    static public void startDownstreamService(Context mContext) {
        Intent intent = new Intent(mContext, DownStream.class);
        mContext.startService(intent);
    }
    static public void goToMainActivity(Context mContext) {
        Intent intent = new Intent(mContext, MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }
    static public void goToRecommendationActivity(Context mContext) {
        Intent intent = new Intent(mContext, RecommendationActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(intent);
    }
    static public boolean isFbLoggedIn(Context mContext) {


        AccessToken accessToken = AccessToken.getCurrentAccessToken();
        return accessToken != null;
    }

    static public boolean isTwitterLoggedIn(Context mContext) {

        TwitterSession session =
                Twitter.getSessionManager().getActiveSession();
        TwitterAuthToken authToken = null;
        if(session!=null)
        {
            authToken = session.getAuthToken();
        }
        if(authToken==null || session ==null)
        {
            Log.v(TAG," Twitter user not logged in");
            return false;
        }
        else
        {
            String token = authToken.token;
            String secret = authToken.secret;
            Log.v(TAG,"Twitter user logged in token="+token+" secret="+secret);
            return true;
        }
    }
    static public String getDateString(Date today ){
        SimpleDateFormat sdf = new SimpleDateFormat("dd-MM-yyyy");
        String date = sdf.format(today);
        return date;
    }
    static public void createNewMobileDateFolder() {


        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                +File.separator+Constants.mobile,
                getDateString(new Date()));
        if(f.exists()==false){
            f.mkdirs();
        }
    }

    static public boolean checkFbJson() {
        Date today = new Date();

        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.mobile + File.separator +
                getDateString(new Date()), Constants.Fb_file);
        if (!f.exists()) {
            return false;
        } else {
            return true;
        }
    }
    static public boolean checkTwitterJson() {


        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.mobile + File.separator +
                getDateString(new Date()), Constants.Twitter_file);
        if (!f.exists()) {
            return false;
        } else {
            return true;
        }
    }
    static public boolean checkChromeJson() {


        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.mobile + File.separator +
                getDateString(new Date()), Constants.Chrome_file);
        if (!f.exists()) {
            return false;
        } else {
            return true;
        }
    }

    static public boolean checkUltimateJson() {


        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.mobile + File.separator +
                getDateString(new Date()), Constants.Ultimate_file);
        if (!f.exists()) {
            return false;
        } else {
            return true;
        }
    }

    static public boolean writeFbJson(String jsonString) {


        boolean result = false;
        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.mobile + File.separator +
                getDateString(new Date()), Constants.Fb_file);
        FileWriter file = null;
        try {
            f.createNewFile();
            file = new FileWriter(f);
            file.write(jsonString);
            Log.v(TAG, "Successfully written todays fb json file");
            result = true;
        } catch (IOException e) {
            Log.v(TAG, "Exception in writing todays fb json file");

        } finally {
            try {
                file.flush();
                file.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        return result;
    }

    static public boolean writeTwitterJson(String jsonString) {


        boolean result = false;
        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.mobile + File.separator +
                getDateString(new Date()), Constants.Twitter_file);
        FileWriter file = null;
        try {
            f.createNewFile();
            file = new FileWriter(f);
            file.write(jsonString);
            Log.v(TAG, "Successfully written todays twitter json file");
            result = true;
        } catch (IOException e) {
            Log.v(TAG, "Exception in writing todays twitter json file");

        } finally {
            try {
                file.flush();
                file.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        return result;
    }
    static public boolean writeChromeJson(String jsonString) {


        boolean result = false;
        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.mobile + File.separator +
                getDateString(new Date()), Constants.Chrome_file);
        FileWriter file = null;
        try {
            f.createNewFile();
            file = new FileWriter(f);
            file.write(jsonString);
            Log.v(TAG, "Successfully written todays chrome json file");
            result = true;
        } catch (IOException e) {
            Log.v(TAG, "Exception in writing todays chrome json file");

        } finally {
            try {
                file.flush();
                file.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        return result;
    }

    static public boolean writeUltimateJson(String jsonString) {


        boolean result = false;
        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.mobile+ File.separator+ getDateString(new Date()),
                Constants.Ultimate_file);
        FileWriter file = null;
        try {
            f.createNewFile();
            file = new FileWriter(f);
            file.write(jsonString);
            Log.v(TAG, "Successfully written todays ultimate json file");
            result = true;
        } catch (IOException e) {
            Log.v(TAG, "Exception in writing todays ultimate json file");

        } finally {
            try {
                file.flush();
                file.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        return result;
    }

    static public boolean writeRecommJson(String date, String jsonString) {


        boolean result = false;
        File folder = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.laptop+ File.separator+ date);
        if(folder.exists()==false){
            folder.mkdirs();
        }
        File f = new File(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                + File.separator + Constants.laptop+ File.separator+ date, Constants.Ultimate_file);
        FileWriter file = null;
        try {
            f.createNewFile();
            file = new FileWriter(f);
            file.write(jsonString);
            Log.v(TAG, "Successfully written todays ultimate json file");
            result = true;
        } catch (IOException e) {
            Log.v(TAG, "Exception in writing todays ultimate json file");

        } finally {
            try {
                file.flush();
                file.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        return result;
    }

    public static String readFile(String filename) {
        String result = null;
        try {
            BufferedReader br = new BufferedReader(new FileReader(filename));
            StringBuilder sb = new StringBuilder();
            String line = br.readLine();
            while (line != null) {
                sb.append(line);
                line = br.readLine();
            }
            result = sb.toString();
        } catch(Exception e) {
            e.printStackTrace();
        }
        return result;
    }

    public static boolean isMyServiceRunning(Context mContext, Class<?> serviceClass) {
        ActivityManager manager = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        for (ActivityManager.RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            if (serviceClass.getName().equals(service.service.getClassName())) {
                return true;
            }
        }
        return false;
    }

    public static ArrayList<RecommendedLinks> getRecommendedLinks(Context mContext){
        ArrayList<RecommendedLinks> listRecomm = null;
        String lastRecvFolder = SharedPreferencesUtil.getStoredPref(mContext, Constants.folderRecieve);
        if(lastRecvFolder!=null){
            String jsonResult = readFile(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR
                    + File.separator + Constants.laptop+ File.separator+ lastRecvFolder+
            File.separator+Constants.Ultimate_file);
            try {
                if (jsonResult != null) {
                    listRecomm = new ArrayList<RecommendedLinks>();
                    JSONObject result = new JSONObject(jsonResult);
                    if(result.has("algo1") && result.getString("algo1")!=null){
                        JSONArray algo1 = (JSONArray)result.getJSONArray("algo1");
                        int count = algo1.length();
                        for(int i = 0; i < count; i++){
                            JSONObject links = algo1.getJSONObject(i);
                            RecommendedLinks listObj = new RecommendedLinks();
                            listObj.setAlgo("algo1");
                            if(links.has("url")){
                                listObj.setUrl(links.getString("url"));
                            }
                            if(links.has("content")){
                                listObj.setContent(links.getString("content"));
                            }
                            if(links.has("use")){
                                listObj.setUse(links.getString("use"));
                            }
                            if(links.has("id")){
                                listObj.setId(links.getString("id"));
                            }
                            if(links.has("title")){
                                listObj.setTitle(links.getString("title"));
                            }
                            listRecomm.add(listObj);
                        }

                    }
                    if(result.has("algo2") && result.getString("algo2")!=null){
                        JSONArray algo2 = (JSONArray)result.getJSONArray("algo2");
                        int count = algo2.length();
                        for(int i = 0; i < count; i++){
                            JSONObject links = algo2.getJSONObject(i);
                            RecommendedLinks listObj = new RecommendedLinks();
                            listObj.setAlgo("algo2");
                            if(links.has("url")){
                                listObj.setUrl(links.getString("url"));
                            }
                            if(links.has("content")){
                                listObj.setContent(links.getString("content"));
                            }
                            if(links.has("use")){
                                listObj.setUse(links.getString("use"));
                            }
                            if(links.has("id")){
                                listObj.setId(links.getString("id"));
                            }
                            if(links.has("title")){
                                listObj.setTitle(links.getString("title"));
                            }
                            listRecomm.add(listObj);
                        }
                    }

                }
            }catch(Exception ex){
                Log.v(TAG,"Exception while retrieving recommended links"+ex.toString());
            }
        }

        return listRecomm;
    }

    public static void openInChrome(Context mContext,String url){
        try {
            Intent i = new Intent("android.intent.action.MAIN");
            i.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            i.setComponent(ComponentName.unflattenFromString("com.android.chrome/com.android.chrome.Main"));
            i.addCategory("android.intent.category.LAUNCHER");
            i.setData(Uri.parse(url));
            mContext.startActivity(i);
        }
        catch(ActivityNotFoundException e) {
            Log.e(TAG,"Chrome not installed");
            // Chrome is probably not installed
        }
    }

    public static void writeFeedbackJson(ArrayList<RecommendedLinks> recLinks, String todayDate){
        JSONObject mainObj = new JSONObject();
        JSONArray linksList = new JSONArray();
        try {
            for (RecommendedLinks linkObj : recLinks) {
                JSONObject jsonObj = new JSONObject();
                jsonObj.put("use", linkObj.getUse());
                jsonObj.put("url", linkObj.getUrl());
                jsonObj.put("content", linkObj.getContent());
                jsonObj.put("title", linkObj.getTitle());
                jsonObj.put("id", linkObj.getId());
                linksList.put(jsonObj);
            }
            mainObj.put("algo1", linksList);
            Util.writeRecommJson(todayDate, mainObj.toString());
        }catch (Exception e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }
}

