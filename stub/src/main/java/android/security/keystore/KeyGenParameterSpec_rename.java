package android.security.keystore;

import java.math.BigInteger;
import java.security.spec.AlgorithmParameterSpec;
import java.util.Date;

import javax.security.auth.x500.X500Principal;

// 放在 stub 模块中，且在包名 android.security.keystore 路径下
// 同时在 xiaxiSafe/src 的 build.gradle 中通过 Android Gradle Plugin (AGP) 提供的 androidComponents API 注册了 onVariants 回调
// ClassVisitorFactory  继承了 AsmClassVisitorFactory，AGP 在编译时会调用它的 createClassVisitor 来生成一个 ASM ClassVisitor
//        ClassRemapper 是 ASM 提供的一个类，用于在字节码层面重写类名，这里重写了 map(String name)：
//        如果类名中有 $（内部类），会先递归处理外部类名再加上 $ 后的部分
//        如果类名是 XXX_rename 结尾，则去掉 _rename
//        其它类不改名
// 而 isInstrumentable 决定只处理 com.xiaxi.safe.util. 包下的使用到带有 _rename 字样的类
// 也就是说在 com.xiaxi.safe.util. 包名下的类中如果使用了 stub 模块中的 KeyGenParameterSpec_rename类，效果如下：
// 编译前 android.security.keystore.KeyGenParameterSpec_rename.Builder(...)
// 编译后 android.security.keystore.KeyGenParameterSpec.Builder(...)
// 这样的好处就是可以绕过 编译期限制，但无法绕过运行期权限/策略，是否可以真正的调用由 keystore2/Keymaster 在 Binder 端做强校验，普通 App 无法规避
public class KeyGenParameterSpec_rename {
    public static class Builder {
        public Builder(String keystoreAlias, int purposes) {
        }

        public Builder setAlgorithmParameterSpec(AlgorithmParameterSpec spec) {
            return this;
        }

        public Builder setCertificateSubject(X500Principal subject) {
            return this;
        }

        public Builder setCertificateSerialNumber(BigInteger serialNumber) {
            return this;
        }

        public Builder setCertificateNotBefore(Date date) {
            return this;
        }

        public Builder setCertificateNotAfter(Date date) {
            return this;
        }

        public Builder setDigests(String... digests) {
            return this;
        }

        public Builder setAttestationChallenge(byte[] attestationChallenge) {
            return this;
        }

        public Builder setDevicePropertiesAttestationIncluded(boolean devicePropertiesAttestationIncluded) {
            return this;
        }

        public Builder setAttestationIds(int[] attestationIds) {
            return this;
        }

        public Builder setUniqueIdIncluded(boolean uniqueIdIncluded) {
            return this;
        }

        public Builder setIsStrongBoxBacked(boolean isStrongBoxBacked) {
            return this;
        }

        public Builder setAttestKeyAlias(String attestKeyAlias) {
            return this;
        }

        public KeyGenParameterSpec_rename build() {
            throw new RuntimeException("Stub!");
        }
    }
}
