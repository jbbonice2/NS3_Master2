import 'package:get/get.dart';

import '../../../../presentation/menu/controllers/menu.controller.dart';

class MenuControllerBinding extends Bindings {
  @override
  void dependencies() {
    Get.lazyPut<MyMenuController>(
      () => MyMenuController(),
    );
  }
}
