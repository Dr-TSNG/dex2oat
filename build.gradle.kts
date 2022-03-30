buildscript {
    repositories {
        google()
        mavenCentral()
    }
    dependencies {
        classpath("com.android.tools.build:gradle:7.1.2")
    }
}

tasks.register<Delete>("clean") {
    delete(rootProject.buildDir)
}
