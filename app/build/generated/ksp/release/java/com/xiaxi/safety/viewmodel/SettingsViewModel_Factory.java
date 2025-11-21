package com.xiaxi.safety.viewmodel;

import android.content.Context;
import com.xiaxi.safety.repository.AppSettingsRepository;
import dagger.internal.DaggerGenerated;
import dagger.internal.Factory;
import dagger.internal.Provider;
import dagger.internal.Providers;
import dagger.internal.QualifierMetadata;
import dagger.internal.ScopeMetadata;
import javax.annotation.processing.Generated;

@ScopeMetadata
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
public final class SettingsViewModel_Factory implements Factory<SettingsViewModel> {
  private final Provider<AppSettingsRepository> appSettingsRepositoryProvider;

  private final Provider<Context> contextProvider;

  public SettingsViewModel_Factory(Provider<AppSettingsRepository> appSettingsRepositoryProvider,
      Provider<Context> contextProvider) {
    this.appSettingsRepositoryProvider = appSettingsRepositoryProvider;
    this.contextProvider = contextProvider;
  }

  @Override
  public SettingsViewModel get() {
    return newInstance(appSettingsRepositoryProvider.get(), contextProvider.get());
  }

  public static SettingsViewModel_Factory create(
      javax.inject.Provider<AppSettingsRepository> appSettingsRepositoryProvider,
      javax.inject.Provider<Context> contextProvider) {
    return new SettingsViewModel_Factory(Providers.asDaggerProvider(appSettingsRepositoryProvider), Providers.asDaggerProvider(contextProvider));
  }

  public static SettingsViewModel_Factory create(
      Provider<AppSettingsRepository> appSettingsRepositoryProvider,
      Provider<Context> contextProvider) {
    return new SettingsViewModel_Factory(appSettingsRepositoryProvider, contextProvider);
  }

  public static SettingsViewModel newInstance(AppSettingsRepository appSettingsRepository,
      Context context) {
    return new SettingsViewModel(appSettingsRepository, context);
  }
}
