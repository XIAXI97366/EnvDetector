package com.xiaxi.safety.datastore.di;

import android.content.Context;
import androidx.datastore.core.DataStore;
import com.xiaxi.safety.datastore.AppSettingsSerializer;
import com.xiaxi.safety.datastore.model.AppSettings;
import dagger.internal.DaggerGenerated;
import dagger.internal.Factory;
import dagger.internal.Preconditions;
import dagger.internal.Provider;
import dagger.internal.Providers;
import dagger.internal.QualifierMetadata;
import dagger.internal.ScopeMetadata;
import javax.annotation.processing.Generated;

@ScopeMetadata("javax.inject.Singleton")
@QualifierMetadata("dagger.hilt.android.qualifiers.ApplicationContext")
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
public final class DataStoreModule_ProvidesUserPreferencesDataStoreFactory implements Factory<DataStore<AppSettings>> {
  private final Provider<Context> contextProvider;

  private final Provider<AppSettingsSerializer> appSettingsSerializerProvider;

  public DataStoreModule_ProvidesUserPreferencesDataStoreFactory(Provider<Context> contextProvider,
      Provider<AppSettingsSerializer> appSettingsSerializerProvider) {
    this.contextProvider = contextProvider;
    this.appSettingsSerializerProvider = appSettingsSerializerProvider;
  }

  @Override
  public DataStore<AppSettings> get() {
    return providesUserPreferencesDataStore(contextProvider.get(), appSettingsSerializerProvider.get());
  }

  public static DataStoreModule_ProvidesUserPreferencesDataStoreFactory create(
      javax.inject.Provider<Context> contextProvider,
      javax.inject.Provider<AppSettingsSerializer> appSettingsSerializerProvider) {
    return new DataStoreModule_ProvidesUserPreferencesDataStoreFactory(Providers.asDaggerProvider(contextProvider), Providers.asDaggerProvider(appSettingsSerializerProvider));
  }

  public static DataStoreModule_ProvidesUserPreferencesDataStoreFactory create(
      Provider<Context> contextProvider,
      Provider<AppSettingsSerializer> appSettingsSerializerProvider) {
    return new DataStoreModule_ProvidesUserPreferencesDataStoreFactory(contextProvider, appSettingsSerializerProvider);
  }

  public static DataStore<AppSettings> providesUserPreferencesDataStore(Context context,
      AppSettingsSerializer appSettingsSerializer) {
    return Preconditions.checkNotNullFromProvides(DataStoreModule.INSTANCE.providesUserPreferencesDataStore(context, appSettingsSerializer));
  }
}
