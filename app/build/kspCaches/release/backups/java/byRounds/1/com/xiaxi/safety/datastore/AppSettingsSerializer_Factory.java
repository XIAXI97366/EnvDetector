package com.xiaxi.safety.datastore;

import dagger.internal.DaggerGenerated;
import dagger.internal.Factory;
import dagger.internal.QualifierMetadata;
import dagger.internal.ScopeMetadata;
import javax.annotation.processing.Generated;

@ScopeMetadata
@QualifierMetadata
@DaggerGenerated
@Generated(
    value = "dagger.internal.codegen.ComponentProcessor",
    comments = "https://dagger.dev"
)
@SuppressWarnings({
    "unchecked",
    "rawtypes",
    "KotlinInternal",
    "KotlinInternalInJava",
    "cast",
    "deprecation",
    "nullness:initialization.field.uninitialized"
})
public final class AppSettingsSerializer_Factory implements Factory<AppSettingsSerializer> {
  @Override
  public AppSettingsSerializer get() {
    return newInstance();
  }

  public static AppSettingsSerializer_Factory create() {
    return InstanceHolder.INSTANCE;
  }

  public static AppSettingsSerializer newInstance() {
    return new AppSettingsSerializer();
  }

  private static final class InstanceHolder {
    static final AppSettingsSerializer_Factory INSTANCE = new AppSettingsSerializer_Factory();
  }
}
