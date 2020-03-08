package ru.a05vip.cf;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.TextView;
import java.net.URL;

import static android.provider.ContactsContract.CommonDataKinds.Website.URL;

public class PhoneReceiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent)
    {
        // TODO: This method is called when the BroadcastReceiver is receiving
        // an Intent broadcast.
        Log.i("HABLA", "OnReceive");

        try
        {
            String state = intent.getStringExtra(TelephonyManager.EXTRA_STATE);
            String incomingNumber = intent.getStringExtra(TelephonyManager.EXTRA_INCOMING_NUMBER);
            if (state.equals(TelephonyManager.EXTRA_STATE_RINGING))
            {
                //Toast.makeText(context, "Ringing State Number is - " + incomingNumber, Toast.LENGTH_SHORT).show();
                String contactName = getContactName(incomingNumber, context);
                //Toast.makeText(context, "Contact name is - " + contactName, Toast.LENGTH_SHORT).show();
                GetBackground bg = new GetBackground(context);
                URL url = new URL("http://www.site.ru/crm-get.php?phone=" + incomingNumber.substring(1));
                URL[] taskParams = new URL[]{url};

                // *** AsyncTask Run ***
                bg.execute(taskParams);
                // *********************
            }
        }
        catch (Exception e)
        {
            System.out.println("HABLA Exception" + e);
        }
    }

    // get contact name from addrbook by ringing number
    private String getContactName(final String incomingNumber, Context context)
    {
        Uri uri=Uri.withAppendedPath(ContactsContract.PhoneLookup.CONTENT_FILTER_URI,Uri.encode(incomingNumber));

        String[] projection = new String[]{ContactsContract.PhoneLookup.DISPLAY_NAME};

        String contactName="";
        Cursor cursor=context.getContentResolver().query(uri,projection,null,null,null);

        if (cursor != null)
        {
            if(cursor.moveToFirst())
            {
                contactName=cursor.getString(0);
            }
            cursor.close();
        }

        return contactName;
    }

    // not work in broadcast receiver!
    //TODO: Delete this
    private void setText(String fromServer, Context context)
    {
        Activity activity = (Activity) context;
        activity.setContentView(R.layout.activity_main);
        TextView t=new TextView(context);

        t=(TextView)activity.findViewById(R.id.TextView01);
        t.setText(fromServer);
    }


}
