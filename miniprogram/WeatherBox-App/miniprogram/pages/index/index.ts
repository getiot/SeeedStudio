// index.ts
// 获取应用实例
const app = getApp<IAppOption>()

var mqtt = require('../../utils/mqtt.min.js');
const crypto = require('../../utils/hex_hmac_sha1.js');

const host = 'broker-cn.emqx.io'

Page({
  data: {
    client: null,
    reconnectCounts: 0,
    options: {
      protocolVersion: 5,
      clientId: 'wiot-01',
      clean: false,
      reconnectPeriod: 1000,
      connectTimeout: 30 * 1000,
      resubscribe: true
    },
    air: {
      temp: {value: 0},
      humi: {value: 0},
      pm25: {value: 0},
      aqi: {value: 0}
    }
  },
  // 事件处理函数
  bindViewTap() {
    wx.navigateTo({
      url: '../logs/logs',
    })
  },
  onLoad() {
    // @ts-ignore
    this.data.client = mqtt.connect(host, this.data.options);
    this.data.client.on('connect', function(connack) {
      wx.showToast({
        title:'连接成功'
      })
    })
  },
  getUserProfile() {
    // 推荐使用wx.getUserProfile获取用户信息，开发者每次通过该接口获取用户个人信息均需用户确认，开发者妥善保管用户快速填写的头像昵称，避免重复弹窗
    wx.getUserProfile({
      desc: '展示用户信息', // 声明获取用户个人信息后的用途，后续会展示在弹窗中，请谨慎填写
      success: (res) => {
        console.log(res)
        this.setData({
          userInfo: res.userInfo,
          hasUserInfo: true
        })
      }
    })
  },
  getUserInfo(e: any) {
    // 不推荐使用getUserInfo获取用户信息，预计自2021年4月13日起，getUserInfo将不再弹出弹窗，并直接返回匿名的用户个人信息
    console.log(e)
    this.setData({
      userInfo: e.detail.userInfo,
      hasUserInfo: true
    })
  }
})
