package com.prometasv.multiplechat_client_android;

import android.os.StrictMode;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.io.IOException;
import java.io.*;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

import static android.os.StrictMode.*;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {



    private Socket apConnSocket = null;
    private InputStream  is;
    private OutputStream os;
    SocketThread thrSockConn;

    TextView textView;
    EditText nameInput, msgInput;
    Button   sendBtn;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ThreadPolicy policy = new ThreadPolicy.Builder().permitAll().build();
        setThreadPolicy(policy);

        textView  = findViewById(R.id.main_text_view);
        nameInput = findViewById(R.id.main_input_name);
        msgInput  = findViewById(R.id.main_input_msg);
        sendBtn   = findViewById(R.id.main_btn_send);

        sendBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                try {
                    byte[] headerBytes = new byte[6];

                    headerBytes[0] = 0x00;
                    headerBytes[1] = 0x01;

                    byte[] nameBytes = nameInput.getText().toString().getBytes("EUC-KR");
                    byte[] msgBytes = msgInput.getText().toString().getBytes("EUC-KR");

                    headerBytes[2] = (byte) ((nameBytes.length + msgBytes.length + 4) >> 8);
                    headerBytes[3] = (byte) ((nameBytes.length + msgBytes.length + 4) >> 16);
                    headerBytes[4] = (byte) ((nameBytes.length + msgBytes.length + 4) >> 24);
                    headerBytes[5] = (byte) ((nameBytes.length + msgBytes.length + 4) >> 32);

                    byte[] dataBytes = new byte[nameBytes.length + msgBytes.length + 4];
                    int pos = 0;

                    dataBytes[pos++] = (byte) (nameBytes.length >> 24);
                    dataBytes[pos++] = (byte) (nameBytes.length >> 32);
                    for (byte iter : nameBytes) {
                        dataBytes[pos++] = iter;
                    }

                    dataBytes[pos++] = (byte)(msgBytes.length >> 24);
                    dataBytes[pos++] = (byte)(msgBytes.length >> 32);

                    for(byte iter : msgBytes){
                        dataBytes[pos++] = iter;
                    }

                    textView.setText(textView.getText() + "\n" + msgInput.getText().toString());

                    try {
                        os.write(headerBytes);
                        os.write(dataBytes);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                } catch (UnsupportedEncodingException e) {
                    e.printStackTrace();
                }
            }
        });

        thrSockConn = new SocketThread();
        thrSockConn.setDaemon(true);
        thrSockConn.start();
    }

    class SocketThread extends Thread{
        private RecvThread recvThread;
        public void run(){
            String ipaddr = "220.118.32.74";
            int    ipport = 8864;

            try {
                apConnSocket = new Socket(ipaddr, ipport);

                is = apConnSocket.getInputStream();
                os = apConnSocket.getOutputStream();

                recvThread = new RecvThread();
                recvThread.setDaemon(true);
                recvThread.start();
            } catch (UnknownHostException ue) {
                System.out.println(ue);
                ue.printStackTrace();
            } catch (IOException ie) {
                System.out.println(ie);
                ie.printStackTrace();
            }
        }

        class RecvThread extends Thread{
            @Override
            public void run() {
                while (true) {
                    try {
                        byte[] header = new byte[6];
                        is.read(header);

                        int dataLen = (
                                (((int) header[2] & 0xff) << 24) |
                                        (((int) header[3] & 0xff) << 16) |
                                        (((int) header[4] & 0xff) << 8) |
                                        (((int) header[5] & 0xff))
                        );

                        byte[] data = new byte[dataLen];
                        is.read(data);

                        int pos = 0;
                        int nameLen = (
                                (((int) data[pos] & 0xff) << 8) |
                                        (((int) data[pos + 1] & 0xff))
                        );
                        pos += 2;

                        final String name = new String(data, pos, nameLen, "EUC-KR");
                        pos += nameLen;

                        int msgLen = (
                                (((int) data[pos] & 0xff) << 8) |
                                        (((int) data[pos + 1] & 0xff))
                        );
                        pos += 2;

                        final String msg = new String(data, pos, msgLen, "EUC-KR");

                        runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                textView.setText(textView.getText() + "\n" + name + " : " + msg);
                            }
                        });

                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }
}
