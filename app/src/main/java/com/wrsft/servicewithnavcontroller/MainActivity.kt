package com.wrsft.servicewithnavcontroller

import android.app.ActivityManager
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Toast
import androidx.lifecycle.ViewModelProvider
import com.wrsft.servicewithnavcontroller.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {


    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        Toast.makeText(this, "onCreate MainActiity", Toast.LENGTH_LONG).show()


        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        //setContentView(R.layout.activity_main)

        //if(! isServiceRunning()){
            // make start service button visible and disable stop service
        //}

        val viewModel = ViewModelProvider(this).get(ViewModel::class.java)
        viewModel.setContext(this)

        //binding.viewmodel = viewModel
    }


}