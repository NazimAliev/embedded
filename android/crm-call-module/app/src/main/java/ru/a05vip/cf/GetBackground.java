package ru.a05vip.cf;

import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.util.Log;
import android.view.Gravity;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.SocketTimeoutException;

import static java.security.AccessController.getContext;
import static ru.a05vip.cf.MyActivity.*;

// In order to perform HTTP operation, we should split this in separate class
// Parse params:
// URL for doInBackground()
// Void for onPreExecute() - not used
// Void for onPostExecute() - not used
// Context passed via constructor
public class GetBackground extends AsyncTask<URL, Void, Void>
{
    URL m_url;
    Context m_context;
    String m_res;

    public GetBackground(Context context)
    {
        m_context = context;
    }
    @Override
    protected void onPreExecute()
    {
        super.onPreExecute();
        //tvInfo.setText("Begin");
    }

    @Override
    protected Void doInBackground(URL...params)
    {
        Log.i("HABLA","Enter to doInBackground");
        m_url = params[0];
        Log.i("HABLA","Got URL: " + m_url.toString());

        // *** call function to HTTP GET ***
        m_res = getDB();
        // *********************************

        Intent my_intent = new Intent(m_context, MyActivity.class);
        my_intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        // pass data to MyActivity to show
        my_intent.putExtra("dbdata", m_res);
        m_context.startActivity(my_intent);

        return null;
    }

    protected void onPostExecute(Void result)
    {
        Log.i("HABLA","onPostExecute: " + result);
        //super.onPostExecute(result);
        Toast toast = Toast.makeText(m_context, m_res, Toast.LENGTH_SHORT);
        TextView tv = (TextView) toast.getView().findViewById(android.R.id.message);
        if( tv != null)
        {
            tv.setGravity(Gravity.CENTER);
        }
        toast.show();
        //tvInfo.setText("End");

        // I know only this method to finish another activity
        ((MyActivity) m_context).finish();
    }

    // main class function
    // connect to m_url HTTP addr, send GET request and receive result string to show
    // result string will show in other activity
    private String getDB()
    {
        Log.i("HABLA","Get string: " + m_url.toString());
        HttpURLConnection urlConnection = null;
        StringBuffer response = new StringBuffer();
        try
        {
            urlConnection = (HttpURLConnection) m_url.openConnection();
            urlConnection.setRequestMethod("GET");
            urlConnection.setRequestProperty("User-Agent", "Mozilla/5.0" );
            urlConnection.setRequestProperty("Accept-Language", "ru-RU,ru");
            urlConnection.setRequestProperty("Content-Type", "text/html; charset=UTF-8");

            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
            String inputLine;

            while ((inputLine = bufferedReader.readLine()) != null)
            {
                response.append(inputLine);
                response.append("\n");
            }
            bufferedReader.close();

            Log.i("HABLA","Response string: " + response.toString());

        }
        catch (IOException e)
        {
            System.out.println("HABLA IOException 3" + e);
        }

        String result = response.toString();
        if(result == "")
        {
            result = "No data in Arnica";
        }
        return result;

    }

    // keep this for future
    private String UdpStaff(String data)
    {
        DatagramSocket s;
        String fromServer = "";

        try
        {
            s = new DatagramSocket();
            byte[] buf = new byte[1000];
            DatagramPacket dp = new DatagramPacket(buf, buf.length);
            InetAddress hostAddress = InetAddress.getByName("192.168.1.131");

            buf = data.getBytes();

            DatagramPacket out = new DatagramPacket(buf, buf.length, hostAddress, 9999);
            s.send(out);

            s.setSoTimeout(10000);   // set the timeout in millisecounds.


            try
            {
                s.receive(dp);
                String rcvd = "HABLA rcvd from " + dp.getAddress() + ", " + dp.getPort() + ": " + new String(dp.getData(), 0, dp.getLength());
                System.out.println(rcvd);
                fromServer = rcvd;
                Log.i("HABLA rcvd", rcvd);
            }
            catch (SocketTimeoutException e)
            {
                System.out.println("HABLA SocketTimeoutException" + e);
            }
            catch (IOException e)
            {
                System.out.println("HABLA IOException 1" + e);
            }

            //s.leaveGroup(InetAddress.getByName(group));
            s.close();
        }
        catch (SocketException e)
        {
            System.out.println("HABLA SocketException " + e);
        }
        catch (IOException e)
        {
            System.out.println("HABLA IOException 2" + e);
        }
        return fromServer;
    }

}
