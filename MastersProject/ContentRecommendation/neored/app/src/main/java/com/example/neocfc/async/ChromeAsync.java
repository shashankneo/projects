package com.example.neocfc.async;

/**
 * Created by neocfc on 6/24/15.
 */
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.Message;
import android.provider.Browser;
import android.util.Log;

import java.util.Date;

public class ChromeAsync extends AsyncTask<Date, Void, String> {
    Context mContext;
    Handler mHandler;
    private String TAG="ChromeAsync";
    public ChromeAsync(Context context,Handler handler) {
        super();
        mContext = context;
        mHandler=handler;
    }
    @Override
    protected String doInBackground(Date... dateObj) {
        String[] proj = new String[] { Browser.BookmarkColumns.TITLE,Browser.BookmarkColumns.URL };
        Uri uriCustom = Uri.parse("content://com.android.chrome.browser/bookmarks");
        String sel = Browser.BookmarkColumns.BOOKMARK + " = 0"; // 0 = history, 1 = bookmark
        Cursor mCur = mContext.getContentResolver().query(uriCustom, proj, sel, null, null);
        mCur.moveToFirst();

        String title = "";

        String url = "";

        if (mCur.moveToFirst() && mCur.getCount() > 0) {
            boolean cont = true;
            while (mCur.isAfterLast() == false && cont) {
                title = mCur.getString(mCur.getColumnIndex(Browser.BookmarkColumns.TITLE));
                url = mCur.getString(mCur.getColumnIndex(Browser.BookmarkColumns.URL));
                Log.v(TAG,"Title ="+title+" url="+url);
                // Do something with title and url
                mCur.moveToNext();
            }
        }
        return "yes";
    }

    @Override
    protected void onPostExecute(String result) {
        Log.v(TAG,"in LoginUserTask postExecute");
        Message msg = Message.obtain();
        msg.obj=result;
        mHandler.sendMessage(msg);
        // textView.setText(result);
    }
}