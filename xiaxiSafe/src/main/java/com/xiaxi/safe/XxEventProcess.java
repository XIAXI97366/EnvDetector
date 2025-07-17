package com.xiaxi.safe;

/**
 * 信息回調接口
 *
 * @author mrack
 */
public interface XxEventProcess {
    /**
     * 異常會回調信息
     *
     * @param code 回調信息
     */
    void onMessage(int code);
}
