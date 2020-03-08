package ru.a05vip.cf;

import android.os.Bundle;
import android.app.Activity;
import android.widget.TextView;

public class MyActivity extends Activity
{
    TextView textViewPhone;
    String data;
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_my);
        Bundle extras = getIntent().getExtras();
        if (extras != null)
        {
            data = extras.getString("dbdata");
        }
        textViewPhone = (TextView)findViewById(R.id.textViewPhone);
        textViewPhone.setText(data);
    }

}
