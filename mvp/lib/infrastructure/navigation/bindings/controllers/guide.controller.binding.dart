import 'package:get/get.dart';

import '../../../../presentation/guide/controllers/guide.controller.dart';

class GuideControllerBinding extends Bindings {
  @override
  void dependencies() {
    Get.lazyPut<GuideController>(
      () => GuideController(),
    );
  }
}
