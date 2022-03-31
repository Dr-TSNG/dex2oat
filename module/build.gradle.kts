import org.apache.commons.codec.binary.Hex
import java.security.MessageDigest

plugins {
    id("com.android.application")
}

val verCode = 1
val verName = "1.0.0"
val moduleId = "icu.nullptr.dex2oat"

dependencies {
    implementation("dev.rikka.ndk.thirdparty:cxx:1.2.0")
}

android {
    compileSdk = 32
    ndkVersion = "24.0.8215888"

    buildFeatures {
        prefab = true
    }

    defaultConfig {
        minSdk = 28
        targetSdk = 32
        versionCode = verCode
        versionName = verName
    }

    externalNativeBuild {
        ndkBuild {
            path("src/main/jni/Android.mk")
        }
    }
}

fun afterEval() = android.applicationVariants.forEach { variant ->
    val variantCapped = variant.name.capitalize()
    val variantLowered = variant.name.toLowerCase()

    val magiskDir = "$buildDir/magisk/$variantLowered"
    val zipFileName = "dex2oat-v$verName-$verCode-$variantLowered.zip"

    val prepareMagiskFilesTask = task<Sync>("prepareMagiskFiles$variantCapped") {
        dependsOn("assemble$variantCapped")

        into(magiskDir)
        from("${rootProject.projectDir}/README.md")
        from("$projectDir/magisk_module") {
            exclude("module.prop")
        }
        from("$projectDir/magisk_module") {
            include("module.prop")
            expand(
                "moduleId" to moduleId,
                "versionName" to "v$verName",
                "versionCode" to verCode,
            )
        }
        into("lib") {
            from("$buildDir/intermediates/ndkbuild/$variantLowered/obj/local")
        }

        doLast {
            fileTree(magiskDir).visit {
                if (isDirectory) return@visit
                val md = MessageDigest.getInstance("SHA-256")
                file.forEachBlock(4096) { bytes, size ->
                    md.update(bytes, 0, size)
                }
                file(file.path + ".sha256").writeText(Hex.encodeHexString(md.digest()))
            }
        }
    }

    val zipTask = task<Zip>("zip${variantCapped}") {
        dependsOn(prepareMagiskFilesTask)
        archiveFileName.set(zipFileName)
        destinationDirectory.set(file("$projectDir/build/outputs/zip"))
        from(magiskDir)
    }

    val adb: String = androidComponents.sdkComponents.adb.get().asFile.absolutePath
    val pushTask = task<Exec>("push${variantCapped}") {
        dependsOn(zipTask)
        workingDir("$projectDir/build/outputs/zip")
        commandLine(adb, "push", zipFileName, "/data/local/tmp/")
    }
    val flashTask = task<Exec>("flash${variantCapped}") {
        dependsOn(pushTask)
        commandLine(
            adb, "shell", "su", "-c",
            "magisk --install-module /data/local/tmp/$zipFileName"
        )
    }
    task<Exec>("flashAndReboot${variantCapped}") {
        dependsOn(flashTask)
        commandLine(adb, "shell", "reboot")
    }
}

afterEvaluate {
    afterEval()
}
