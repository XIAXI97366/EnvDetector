package com.example.checkrom;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.os.Environment;
import android.text.TextUtils;
import android.util.Log;
import android.widget.TextView;

import com.example.checkrom.databinding.ActivityMainBinding;
import com.xiaxi.safe.XxSafe;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'checkrom' library on application startup.
    static {
        System.loadLibrary("checkrom");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        //if(new File("/data/data/com.topjohnwu.magisk").exists()) {}

        // Example of a call to a native method
        TextView tv = binding.sampleText;
        tv.setText(stringFromJNI());
    }



    /**
     * A native method that is implemented by the 'checkrom' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}