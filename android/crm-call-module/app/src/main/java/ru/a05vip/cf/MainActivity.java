package ru.a05vip.cf;

import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity
{
    String versionName;
    TextView textViewVersion;
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        PackageManager manager = this.getPackageManager();
        PackageInfo info = null;
        try
        {
            info = manager.getPackageInfo(this.getPackageName(), 0);
        }
        catch (PackageManager.NameNotFoundException e)
        {
            e.printStackTrace();
        }
        versionName = info.versionName;

        setContentView(R.layout.activity_main);
        textViewVersion = (TextView)findViewById(R.id.textViewVersion);
        textViewVersion.setText(versionName);
    }
}
