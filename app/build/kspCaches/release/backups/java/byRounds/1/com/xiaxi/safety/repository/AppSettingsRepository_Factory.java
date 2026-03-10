package com.xiaxi.safety.repository;

import com.xiaxi.safety.datastore.AppSettingsDataStore;
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
public final class AppSettingsRepository_Factory implements Factory<AppSettingsRepository> {
  private final Provider<AppSettingsDataStore> userPreferencesDataSourceProvider;

  public AppSettingsRepository_Factory(
      Provider<AppSettingsDataStore> userPreferencesDataSourceProvider) {
    this.userPreferencesDataSourceProvider = userPreferencesDataSourceProvider;
  }

  @Override
  public AppSettingsRepository get() {
    return newInstance(userPreferencesDataSourceProvider.get());
  }

  public static AppSettingsRepository_Factory create(
      javax.inject.Provider<AppSettingsDataStore> userPreferencesDataSourceProvider) {
    return new AppSettingsRepository_Factory(Providers.asDaggerProvider(userPreferencesDataSourceProvider));
  }

  public static AppSettingsRepository_Factory create(
      Provider<AppSettingsDataStore> userPreferencesDataSourceProvider) {
    return new AppSettingsRepository_Factory(userPreferencesDataSourceProvider);
  }

  public static AppSettingsRepository newInstance(AppSettingsDataStore userPreferencesDataSource) {
    return new AppSettingsRepository(userPreferencesDataSource);
  }
}
