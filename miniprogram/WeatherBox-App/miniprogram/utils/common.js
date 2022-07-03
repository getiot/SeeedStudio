const news = [
  {
    id: '264698',
    title: '俄罗斯联邦驻华大使杰尼索夫会见校党委书记顾家山一行并接受《力冈译文全集》赠予',
    poster: 'http://www.ahnu.edu.cn/__local/A/C7/68/C2C9E5E2161A466A2D54D21A63C_DD3FEC40_4EBBB.jpg?e=.jpg',
    content: '新华社 3月2日上午，俄罗斯联邦驻华大使杰尼索夫会见校党委书记顾家山一行并接受《力冈译文全集》赠予。',
    add_date: '2018-03-05'
  },
  {
    id: '305670',
    title: '我校学子在全省第八届少数民族传统体育运动会上喜获佳绩',
    poster: 'http://www.ahnu.edu.cn/__local/7/CC/BD/47349A7168770AC24EB5535B2AF_35829829_6E24D.png?e=.png',
    content: '新华社 11月26日上午，我校学子在全省第八届少数民族传统体育运动会上喜获佳绩。',
    add_date: '2018-11-27'
  },
  {
    id: '304083',
    title: '我校学子代表国家队获中国羽毛球公开赛男双亚军',
    poster: 'http://www.ahnu.edu.cn/__local/2/BB/2D/9FEB9B8D0CA5E059B2C1C7E65BD_67A0410C_3A71F.png?e=.png',
    content: '新华社 11月12日晚上，我校学子代表国家队获中国羽毛球公开赛男双亚军。',
    add_date: '2018-11-14'
  },
];

/**
 * 获取新闻列表
 */
function getNewsList() {
  let list = [];
  for (var i=0; i<news.length; i++) {
    let obj = {};
    obj.id = news[i].id;
    obj.poster = news[i].poster;
    obj.add_date = news[i].add_date;
    obj.title = news[i].title;
    list.push(obj);
  }
  return list;
}

/**
 * 获取新闻内容
 */
function getNewsDetail (newsID) {
  let msg = {
    code: '404',
    news: {}
  };
  for (var i = 0; i < news.length; i++) {
    if (newsID == news[i].id) {
      msg.code = '200';
      msg.news = news[i];
      break;
    }
  }
  return msg;
}

module.exports = {
  getNewsList: getNewsList,
  getNewsDetail: getNewsDetail
}