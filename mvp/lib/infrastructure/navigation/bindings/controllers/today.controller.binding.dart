import 'package:get/get.dart';

import '../../../../presentation/today/controllers/today.controller.dart';

class TodayControllerBinding extends Bindings {
  @override
  void dependencies() {
    Get.lazyPut<TodayController>(
      () => TodayController(),
    );
  }
}
