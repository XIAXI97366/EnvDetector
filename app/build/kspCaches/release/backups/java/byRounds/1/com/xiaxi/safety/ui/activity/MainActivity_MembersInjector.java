package com.xiaxi.safety.ui.activity;

import com.xiaxi.safety.repository.AppSettingsRepository;
import dagger.MembersInjector;
import dagger.internal.DaggerGenerated;
import dagger.internal.InjectedFieldSignature;
import dagger.internal.Provider;
import dagger.internal.Providers;
import dagger.internal.QualifierMetadata;
import javax.annotation.processing.Generated;

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
public final class MainActivity_MembersInjector implements MembersInjector<MainActivity> {
  private final Provider<AppSettingsRepository> appSettingsRepositoryProvider;

  public MainActivity_MembersInjector(
      Provider<AppSettingsRepository> appSettingsRepositoryProvider) {
    this.appSettingsRepositoryProvider = appSettingsRepositoryProvider;
  }

  public static MembersInjector<MainActivity> create(
      Provider<AppSettingsRepository> appSettingsRepositoryProvider) {
    return new MainActivity_MembersInjector(appSettingsRepositoryProvider);
  }

  public static MembersInjector<MainActivity> create(
      javax.inject.Provider<AppSettingsRepository> appSettingsRepositoryProvider) {
    return new MainActivity_MembersInjector(Providers.asDaggerProvider(appSettingsRepositoryProvider));
  }

  @Override
  public void injectMembers(MainActivity instance) {
    injectAppSettingsRepository(instance, appSettingsRepositoryProvider.get());
  }

  @InjectedFieldSignature("com.xiaxi.safety.ui.activity.MainActivity.appSettingsRepository")
  public static void injectAppSettingsRepository(MainActivity instance,
      AppSettingsRepository appSettingsRepository) {
    instance.appSettingsRepository = appSettingsRepository;
  }
}
