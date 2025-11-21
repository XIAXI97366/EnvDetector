package com.xiaxi.safe;


public class XxEvent {
    /** Hook框架检测 */
    public final static int HOOK = 1;

    /** 反调试检测 */
    public final static int DEBUG = 2;

    /** Root权限检测 */
    public final static int ROOT = 3;

    /** 模拟器检测 */
    public final static int EMU = 4;

    /** 环境异常检测（如开启VPN、解锁bl） */
    public final static int ENV = 5;

    /** 多开检测 */
    public final static int VA = 6;

    /** 运行时篡改检测（如注入框架） */
    public final static int RUNTIME_TAMPERING = 7;

    /**  APK完整性检测 */
    public final static int APK_INTEGRITY = 8;
}
