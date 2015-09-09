package com.example.neocfc.util;

import android.content.Context;
import android.content.SharedPreferences;

/**
 * Created by neocfc on 6/28/15.
 */
final  public class SharedPreferencesUtil {
    static public String getStoredPref(Context context,String key){
        SharedPreferences prefs = context.getSharedPreferences(Constants.storedPref, 0);
        String value = prefs.getString(key, null);
        return value;
    }
    static public void setStoredPref(Context context, String key,String value){
        SharedPreferences.Editor editor = context.getSharedPreferences(Constants.storedPref, 0).edit();
        editor.putString(key, value);
        editor.commit();
    }
}
