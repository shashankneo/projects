package com.example.neocfc.util;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Browser;
import android.util.Log;


import com.facebook.AccessToken;
import com.facebook.FacebookRequestError;
import com.facebook.GraphRequest;
import com.facebook.GraphResponse;
import com.facebook.HttpMethod;
import com.twitter.sdk.android.Twitter;
import com.twitter.sdk.android.core.Callback;
import com.twitter.sdk.android.core.Result;
import com.twitter.sdk.android.core.TwitterApiClient;
import com.twitter.sdk.android.core.TwitterException;
import com.twitter.sdk.android.core.TwitterSession;
import com.twitter.sdk.android.core.models.Tweet;
import com.twitter.sdk.android.core.models.TweetEntities;
import com.twitter.sdk.android.core.models.UrlEntity;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.simple.parser.JSONParser;

import java.io.File;
import java.io.FileReader;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.List;
import java.util.Set;

import retrofit.http.GET;

/**
 * Created by neocfc on 7/1/15.
 */
public class JsonUtil {
    public static String TAG = "JsonUtil";
    private boolean fb_json_created = false;
    private boolean twitter_json_created = false;
    public boolean getFbJson(){
        JsonData.fb_prep = false;
        AccessToken accessToken = AccessToken.getCurrentAccessToken();
        String token = accessToken.getToken();
        Date expiry_date = accessToken.getExpires();
        Set<String> granted_permissions = accessToken.getPermissions();
        Set<String> declined_permissions = accessToken.getDeclinedPermissions();
        Log.v(TAG, "Token=" + token + " date=" + expiry_date.toString() + "perm-" + granted_permissions.toString()
                + "declined=" + declined_permissions);
        Bundle params = new Bundle();
        params.putString("filter", "nf");
            /* make the API call */
        new GraphRequest(
                accessToken,
                "/me/feed",
                null,
                HttpMethod.GET,
                new GraphRequest.Callback() {
                    public void onCompleted(GraphResponse response) {
                        FacebookRequestError reqError = response.getError();
                        if (reqError != null) {
                            Log.v(TAG, "Error code=" + reqError.getErrorCode() + " error_type=" + reqError.getErrorType()
                                    + " error_msg=" + reqError.getErrorMessage());

                        }
                        else{
                            JSONObject dataJson = response.getJSONObject();
                            JSONObject final_json = new JSONObject();
                            JSONArray msgList = new JSONArray();
                            JSONArray linksList = new JSONArray();
                            try{
                                JSONObject Obj = dataJson;
                                JSONArray data = Obj.getJSONArray("data");
                                int length = data.length();
                                int i =0;
                                for(i=0;i<length;i++){
                                    JSONObject fbObj = data.getJSONObject(i);
                                    if(fbObj.has("message") && fbObj.getString("message")!=null){
                                        JSONObject fbJson = new JSONObject();
                                        fbJson.put("message",fbObj.getString("message"));
                                        fbJson.put("id",fbObj.getString("id"));
                                        msgList.put(fbJson);
                                    }

                                    if(fbObj.has("link") && fbObj.getString("link")!=null){
                                        JSONObject fbJson = new JSONObject();
                                        fbJson.put("link",fbObj.getString("link"));
                                        linksList.put(fbJson);
                                    }
                                }
                                final_json.put("fb_msg",msgList);
                                final_json.put("links",linksList);

                                boolean result = Util.writeFbJson(final_json.toString());
                                if(result == true){
                                    fb_json_created = true;
                                }

                            }catch(JSONException ex){
                                Log.v(TAG,"Exception in parsing of Facebook feed="+ex.toString());
                            }
                        }


                        //  JsonData.fb_json = response.getJSONObject();
                        //Log.v(TAG, "Raw response=" + JsonData.fb_json);

                        JsonData.fb_prep = false;
                    }
                }
        ).executeAndWait();

        while(JsonData.fb_prep==true);

        return fb_json_created;
    }

    public boolean getTwitterJson(){

        TwitterSession session =
                Twitter.getSessionManager().getActiveSession();
        MyTwitterApiClient twitterApiClient = new MyTwitterApiClient(session);
        CustomService customService = twitterApiClient.getCustomService();
        customService.show( new Callback<List<Tweet>>() {
            @Override
            public void success(Result<List<Tweet>> result) {
                //Do something with result, which provides a Tweet inside of result.data
                List<Tweet> listTweets = result.data;
                JSONObject tweetsJson = new JSONObject();
                JSONArray tweetList = new JSONArray();
                JSONArray linksList = new JSONArray();
                try {
                        for(Tweet obj : listTweets)
                        {

                                if (obj.text != null) {
                                    JSONObject temp = new JSONObject();
                                    temp.put("tweet", obj.text);
                                    tweetList.put(temp);
                                }
                                TweetEntities objentity = obj.entities;
                                if (objentity != null) {
                                    for (UrlEntity urlObj : objentity.urls) {
                                        JSONObject temp = new JSONObject();
                                        temp.put("link", urlObj.expandedUrl);
                                        linksList.put(temp);
                                    }
                                }
                        }
                    tweetsJson.put("tweets",tweetList);
                    tweetsJson.put("links",linksList);

                    boolean twitter_result = Util.writeTwitterJson(tweetsJson.toString());
                    if(twitter_result == true){
                        twitter_json_created = true;
                    }

                }
                catch(JSONException ex){
                    Log.v(TAG,"Exception while parsing todays tweets");
                }
                JsonData.twitter_prep = false;
            }

            public void failure(TwitterException exception) {
                Log.v(TAG, "Timeline not obtained="+exception.toString());
                JsonData.fb_prep = false;
            }
        });

        while(JsonData.twitter_prep==true);

        return twitter_json_created;
    }
    class MyTwitterApiClient extends TwitterApiClient {
        public MyTwitterApiClient(TwitterSession session) {
            super(session);
        }

        /**
         * Provide CustomService with defined endpoints
         */
        public CustomService getCustomService() {
            return getService(CustomService.class);
        }
    }

    // example users/show service endpoint
    interface CustomService {
        @GET("/1.1/statuses/home_timeline.json")
        void show(Callback<List<Tweet>> cb);
    }

    public boolean getChromeJson(Context mContext) {
        boolean chrome_json_created = false;

        // today
        Calendar date = new GregorianCalendar();
        // reset hour, minutes, seconds and millis
        date.set(Calendar.HOUR_OF_DAY, 0);
        date.set(Calendar.MINUTE, 0);
        date.set(Calendar.SECOND, 0);
        date.set(Calendar.MILLISECOND, 0);
        long start_date = date.getTimeInMillis();
        // next day
        date.add(Calendar.DAY_OF_MONTH, 1);
        long end_date = date.getTimeInMillis();

        String[] proj = new String[] { Browser.BookmarkColumns.TITLE,Browser.BookmarkColumns.URL };
        Uri uriCustom = Uri.parse("content://com.android.chrome.browser/bookmarks");
        String sel = Browser.BookmarkColumns.BOOKMARK + " = 0 AND "+Browser.BookmarkColumns.DATE+
                ">="+ start_date + " AND "+Browser.BookmarkColumns.DATE+" <="+end_date;
        Cursor mCur = mContext.getContentResolver().query(uriCustom, proj, sel, null, null);
        mCur.moveToFirst();

        String title = "";
        String url = "";
        JSONArray linkList = new JSONArray();
        JSONObject chromeJson = new JSONObject();

        try {
                if (mCur.moveToFirst() && mCur.getCount() > 0) {
                    boolean cont = true;

                        while (mCur.isAfterLast() == false && cont) {
                            JSONObject urlObj = new JSONObject();
                            title = mCur.getString(mCur.getColumnIndex(Browser.BookmarkColumns.TITLE));
                            url = mCur.getString(mCur.getColumnIndex(Browser.BookmarkColumns.URL));
                            Log.v(TAG, "Title =" + title + " url=" + url);

                            urlObj.put("link", url);
                            linkList.put(urlObj);
                            // Do something with title and url
                            mCur.moveToNext();
                        }
                        chromeJson.put("links",linkList);

                }
                else{
                     chromeJson.put("links",null);
                }
        }catch(JSONException ex){
                Log.v(TAG,"Exception while parsing chrome data = "+ex.toString());
        }
        chrome_json_created = Util.writeChromeJson(chromeJson.toString());
        return chrome_json_created;
    }

    public boolean createUltimateFile(){
        boolean result = false;

        JSONObject chromeJson,fbJson,twitterJson ;
        JSONObject finalJson = new JSONObject();
        JSONArray chromeLinks = null;
        JSONArray fbLinks = null;
        JSONArray twitterLinks = null;
        String data = null;
        JSONParser parser = new JSONParser();
        try{
            data = Util.readFile(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR +
                    File.separator + Constants.mobile + File.separator +
                    Util.getDateString(new Date()) + File.separator + Constants.Chrome_file);
            chromeJson =new JSONObject(data);
            data = Util.readFile(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR +
                    File.separator + Constants.mobile + File.separator +
                    Util.getDateString(new Date()) + File.separator + Constants.Fb_file);
            fbJson =new JSONObject(data);
            data = Util.readFile(Environment.getExternalStorageDirectory() + File.separator + Constants.DIR +
                    File.separator + Constants.mobile + File.separator +
                    Util.getDateString(new Date()) + File.separator + Constants.Twitter_file);
            twitterJson =new JSONObject(data);
            if(fbJson.has("fb_msg")){
            finalJson.put("fb_msg",fbJson.get("fb_msg"));
            }
            else{
                finalJson.put("fb_msg","");
            }
            if(twitterJson.has("tweets")){
            finalJson.put("tweets", twitterJson.get("tweets"));
            }
            else{
                finalJson.put("tweets", "");
            }


             if(chromeJson.has("links")){
                 chromeLinks = (JSONArray)chromeJson.get("links");
             }
             if(fbJson.has("links")){
                 fbLinks = (JSONArray)fbJson.get("links");
             }
             if(twitterJson.has("links")){
                 twitterLinks = (JSONArray)twitterJson.get("links");
             }

            JSONArray linkList = concatUltimateArray(chromeLinks,twitterLinks, fbLinks);
            finalJson.put("links",linkList);

            result = Util.writeUltimateJson(finalJson.toString());
        }catch(Exception ex){
            Log.v(TAG,"Exception while generating ultimate file"+ex.toString());
        }
        return result;
    }

    JSONArray concatUltimateArray(JSONArray... arrs)
            throws JSONException {
        JSONArray result = new JSONArray();
        for (JSONArray arr : arrs) {
            if(arr == null)
                continue;
            for (int i = 0; i < arr.length(); i++) {
                JSONObject objlink = (JSONObject)arr.get(i);
                result.put(objlink);

            }
        }
        return result;
    }
}
