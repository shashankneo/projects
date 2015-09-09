package com.example.neocfc.activity;

import android.app.AlertDialog;
import android.app.ListActivity;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import com.example.neocfc.neored.R;
import com.example.neocfc.util.Constants;
import com.example.neocfc.util.RecommendedLinks;
import com.example.neocfc.util.SharedPreferencesUtil;
import com.example.neocfc.util.Util;
import com.wdullaer.swipeactionadapter.SwipeActionAdapter;
import com.wdullaer.swipeactionadapter.SwipeDirections;

import java.util.ArrayList;
import java.util.Arrays;


public class RecommendationActivity extends ListActivity implements
        SwipeActionAdapter.SwipeActionListener
{
    protected SwipeActionAdapter mAdapter;
    protected RecommendationAdapter recommAdapter;
    private ArrayList<RecommendedLinks> recommLinks;
    private String recommDate = null;
    private String TAG="RecommendationActivity";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (Util.isFbLoggedIn(getApplicationContext()) == false || Util.isTwitterLoggedIn(getApplicationContext()) == false) {
            //Go to for user to provide login credentials
            Log.v(TAG, " Fb/Twitter user is not logged in. Go to Main Activity");
            Util.goToMainActivity(getApplicationContext());
        }
        recommLinks = Util.getRecommendedLinks(getApplicationContext());
        recommDate = SharedPreferencesUtil.getStoredPref(getApplicationContext(), Constants.folderRecieve);
        if (recommLinks != null) {
            recommAdapter = new RecommendationAdapter(getApplicationContext(), recommLinks);
            mAdapter = new SwipeActionAdapter(recommAdapter);
            mAdapter.setSwipeActionListener(this)
                    .setDimBackgrounds(true)
                    .setListView(getListView());
            setListAdapter(mAdapter);

            mAdapter.addBackground(SwipeDirections.DIRECTION_FAR_LEFT, R.layout.row_bg_left_far)
                    .addBackground(SwipeDirections.DIRECTION_NORMAL_LEFT, R.layout.row_bg_left)
                    .addBackground(SwipeDirections.DIRECTION_FAR_RIGHT, R.layout.row_bg_right_far)
                    .addBackground(SwipeDirections.DIRECTION_NORMAL_RIGHT, R.layout.row_bg_right);

        }
        else{
            Toast.makeText(this,"No Recommended links for today.Sorry", Toast.LENGTH_SHORT).show();
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onListItemClick(ListView listView, View view, int position, long id){
        Toast.makeText(
                this,
                "Clicked "+mAdapter.getItem(position),
                Toast.LENGTH_SHORT
        ).show();
        recommClicked(position, "yes");
    }

    public void recommClicked(int position,String use){

        RecommendedLinks recommObj = (RecommendedLinks) recommAdapter.links.get(position);
        recommObj.setUse(use);
        if(use.equalsIgnoreCase("no")){
            recommAdapter.filteredLinks.remove(position);
        }
        else{
            Util.openInChrome(getApplicationContext(),recommObj.getUrl());
        }
        mAdapter.notifyDataSetChanged();
    }

    @Override
    public boolean hasActions(int position){
        return true;
    }

    @Override
    public boolean shouldDismiss(int position, int direction){
        return direction == SwipeDirections.DIRECTION_NORMAL_LEFT ||
                direction == SwipeDirections.DIRECTION_FAR_LEFT ||
                direction == SwipeDirections.DIRECTION_FAR_RIGHT ||
                direction == SwipeDirections.DIRECTION_NORMAL_RIGHT;
    }

    @Override
    public void onSwipe(int[] positionList, int[] directionList){
        for(int i=0;i<positionList.length;i++) {
            int direction = directionList[i];
            int position = positionList[i];
            String dir = "";
            String use = "NA";
            switch (direction) {
                case SwipeDirections.DIRECTION_FAR_LEFT:
                    dir = "Far left";
                    use = "no";
                    break;
                case SwipeDirections.DIRECTION_NORMAL_LEFT:
                    dir = "Left";
                    use = "no";
                    break;
                case SwipeDirections.DIRECTION_FAR_RIGHT:
                    dir = "Far right";
                    use = "yes";
                    break;
                case SwipeDirections.DIRECTION_NORMAL_RIGHT:
                    dir = "Right";
                    use = "yes";
                    break;
            }
            recommClicked(position,use);
        }
    }
    public class RecommendationAdapter extends ArrayAdapter<RecommendedLinks> {
        private  Context context;
        private  ArrayList<RecommendedLinks> links;
        private  ArrayList<RecommendedLinks> filteredLinks;
        public class ViewHolder
        {
            TextView txtContent;
            TextView txtTitle;
            LinearLayout mainLL;
        }
        public RecommendationAdapter(Context context, ArrayList<RecommendedLinks> links) {
            super(context, 0, links);
            this.context = context;
            this.links = links;
            this.filteredLinks = links;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            ViewHolder holder;
            if (convertView == null)
            {
            LayoutInflater inflater = (LayoutInflater) context
                    .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
                convertView = inflater.inflate(R.layout.row_bg, parent, false);
                holder = new ViewHolder();

                holder.txtContent = (TextView) convertView.findViewById(R.id.content);
                holder.txtTitle = (TextView) convertView.findViewById(R.id.title);
                holder.mainLL = (LinearLayout) convertView.findViewById(R.id.mainLL);
                convertView.setTag(holder);
            }else{
                holder = (ViewHolder) convertView.getTag();
            }

            String title = filteredLinks.get(position).getTitle();
            String content = filteredLinks.get(position).getContent();
            if(content.length() > Constants.MAX_CONTENT_LEN){
                content = content.substring(0, Constants.MAX_CONTENT_LEN);
            }
            if(title.length() > Constants.MAX_TITLE_LEN){
                title = title.substring(0, Constants.MAX_TITLE_LEN);
        }
            holder.txtTitle.setText(title);
            holder.txtContent.setText(content);
            return convertView;
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
        if(recommAdapter!=null)
            Util.writeFeedbackJson(recommAdapter.links,recommDate);

    }
}