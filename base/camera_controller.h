#pragma once

#include "nelf/nelf.h"
#include "azer/render/render.h"

class CameraController {
 public:
  CameraController(azer::Camera* camera);
  ~CameraController();

  bool OnKeyPressed(const ui::KeyEvent& event);
  bool OnKeyReleased(const ui::KeyEvent& event);
  bool OnMousePressed(const ui::MouseEvent& event);
  bool OnMouseDragged(const ui::MouseEvent& event);
  bool OnMouseReleased(const ui::MouseEvent& event);
  void Update(const azer::FrameArgs& args);
 private:
  void ResetState();
  void RotateCamera(const gfx::Point& prev, const gfx::Point& cur);

  azer::Camera* camera_;
  azer::Quaternion origin_orient_;
  bool orientation_dragging_;
  bool posx_, posy_, posz_;
  bool negx_, negy_, negz_;
  gfx::Point location_;
  DISALLOW_COPY_AND_ASSIGN(CameraController);
};


class CameraEventListener : public nelf::EventListener {
 public:
  CameraEventListener(azer::Camera* camera) {
    controller_.reset(new CameraController(camera));    
  }
  void Update(const azer::FrameArgs& args) { controller_->Update(args);}
  void OnKeyPressed(const ui::KeyEvent& event) override {
    controller_->OnKeyPressed(event);
  }
  void OnKeyReleased(const ui::KeyEvent& event) override {
    controller_->OnKeyReleased(event);
  }
  void OnMousePressed(const ui::MouseEvent& event) override {
    controller_->OnMousePressed(event);
  }
  void OnMouseDragged(const ui::MouseEvent& event) override {
    controller_->OnMouseDragged(event);
  }
  void OnMouseReleased(const ui::MouseEvent& event) override {
    controller_->OnMouseReleased(event);
  }
 private:
  scoped_ptr<CameraController> controller_;
  DISALLOW_COPY_AND_ASSIGN(CameraEventListener);
};
