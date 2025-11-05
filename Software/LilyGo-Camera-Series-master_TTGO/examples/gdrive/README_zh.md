# gdrive

将 esp32 拍摄的照片上传到 Google Drive。

## 创建

1. 新建项目

![](../../docs/_static/gdrive/creat_apps_script.png)

2. 起一个有意义的项目名称

![](../../docs/_static/gdrive/rename_project.png)

3. 将代码替换为以下内容，并保存

```javascript
function doPost(e) {
  var data = Utilities.base64Decode(e.parameters.data);
  var filename = Utilities.formatDate(new Date(), "GMT+8", "yyyyMMdd_HHmmss")+".jpg";
  var blob = Utilities.newBlob(data, e.parameters.mimetype, filename);

  var folder, folders = DriveApp.getFoldersByName("ESP32-CAM");
  if (folders.hasNext()) {
    folder = folders.next();
  } else {
    folder = DriveApp.createFolder("ESP32-CAM");
  }
  var file = folder.createFile(blob); 

  return ContentService.createTextOutput("Completo.")
}
```

> 更多开发指南，请参考 https://developers.google.com/apps-script/guides/web

## Deploy

1. 进入管理部署页面

    ![](../../docs/_static/gdrive/manage_deployment.png)

2. 新建部署

    ![](../../docs/_static/gdrive/create_deployment.png)

3. 选择部署类型

    ![](../../docs/_static/gdrive/choose_deployment_type.png)

4. 选择权限

    ![](../../docs/_static/gdrive/choose_permissions.png)

5. 授予访问权限

    ![](../../docs/_static/gdrive/granted_permission.png)

    ![](../../docs/_static/gdrive/skip_warning.png)

    ![](../../docs/_static/gdrive/granted_permission_1.png)

6. 获取url

    我们将会获得这个的url: https://script.google.com/macros/s/XXXXXXXXXXXXXX/exec

    ![](../../docs/_static/gdrive/get_url.png)

> 警告
>
> 以上整个操作似乎会导致Google Drive变的不安全，请自行斟酌使用。

## Test

使用Postman测试结果如下:

![](../../docs/_static/gdrive/postman_test.png)

## esp32

需要手动修改 [gdrive.ino](./gdrive.ino) 的第7-9行的值。

将 [gdrive.ino](./gdrive.ino) 的 `myScript` 替换成在 [Deploy](#Deploy) 获取的url, url中的 `https://script.google.com` 不要用填入 `myScript`。

## Thanks

该应用程序的代码来自 [gsampallo](https://github.com/gsampallo/esp32cam-gdrive)。
