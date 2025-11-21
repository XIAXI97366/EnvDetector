package com.xiaxi.safety.datastore;

import androidx.datastore.core.DataStore;
import com.xiaxi.safety.datastore.model.AppSettings;
import dagger.internal.DaggerGenerated;
import dagger.internal.Factory;
import dagger.internal.Provider;
import dagger.internal.Providers;
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
public final class AppSettingsDataStore_Factory implements Factory<AppSettingsDataStore> {
  private final Provider<DataStore<AppSettings>> appSettingsProvider;

  public AppSettingsDataStore_Factory(Provider<DataStore<AppSettings>> appSettingsProvider) {
    this.appSettingsProvider = appSettingsProvider;
  }

  @Override
  public AppSettingsDataStore get() {
    return newInstance(appSettingsProvider.get());
  }

  public static AppSettingsDataStore_Factory create(
      javax.inject.Provider<DataStore<AppSettings>> appSettingsProvider) {
    return new AppSettingsDataStore_Factory(Providers.asDaggerProvider(appSettingsProvider));
  }

  public static AppSettingsDataStore_Factory create(
      Provider<DataStore<AppSettings>> appSettingsProvider) {
    return new AppSettingsDataStore_Factory(appSettingsProvider);
  }

  public static AppSettingsDataStore newInstance(DataStore<AppSettings> appSettings) {
    return new AppSettingsDataStore(appSettings);
  }
}
