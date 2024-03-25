package com.lalilu.fpcalc

import android.os.Bundle
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        GlobalScope.launch(Dispatchers.IO) {
            val result = runCatching {
                val fd = resources.openRawResourceFd(R.raw.test).parcelFileDescriptor.detachFd()

                Fpcalc.calc(arrayOf("-raw", "$fd"))
            }.getOrNull()

            withContext(Dispatchers.Main) {
                this@MainActivity.findViewById<TextView>(R.id.result).text = "$result"
                Toast.makeText(this@MainActivity, result.toString(), Toast.LENGTH_SHORT).show()
            }
        }
    }
}
