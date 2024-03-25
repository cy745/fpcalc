package com.lalilu.fpcalc

import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import com.blankj.utilcode.util.FileIOUtils
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.io.File

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        GlobalScope.launch(Dispatchers.IO) {
            val tempFile = File(cacheDir, "tempAudio")
            val inputStream = resources.openRawResource(R.raw.test)
            FileIOUtils.writeFileFromIS(tempFile, inputStream)
            // 使用openRawResourceFd获取到的fd用于Fpcalc进行读取会导致MediaExtractor创建失败
            // 故先通过读取流后，写入到cache文件夹中，再将该文件的path用于读取测试

            val params = FpcalcParams(targetFilePath = tempFile.path)
            val result = Fpcalc.calc(params)

            withContext(Dispatchers.Main) {
                this@MainActivity.findViewById<TextView>(R.id.result).text = "$result"
            }
        }
    }
}
