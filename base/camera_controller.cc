#include "demo/base/camera_controller.h"

#include "azer/render/render.h"

using namespace azer;

CameraController::CameraController(Camera* camera) 
    : camera_(camera) {
  orientation_dragging_ = false;
  ResetState();
}

CameraController::~CameraController() {
}

void CameraController::ResetState() {
  posx_ = 0;
  posz_ = 0;
  posy_ = 0;
  negx_ = 0;
  negz_ = 0;
  negy_ = 0;
}

bool CameraController::OnKeyPressed(const ui::KeyEvent& event) {
  if (event.key_code() == ui::VKEY_W) {
    posz_ = 1;
    return true;
  } else if (event.key_code() == ui::VKEY_S) {
    negz_ = 1;
    return true;
  } else if (event.key_code() == ui::VKEY_A) {
    negx_ = 1;
    return true;
  } else if (event.key_code() == ui::VKEY_D) {
    posx_ = 1;
    return true;
  } else if (event.key_code() == ui::VKEY_F) {
    posy_ = 1;
    return true;
  } else if (event.key_code() == ui::VKEY_R) {
    negy_ = 1;
    return true;
  } else {
    return false;
  }
}

bool CameraController::OnKeyReleased(const ui::KeyEvent& event) {
  if (event.key_code() == ui::VKEY_W) {
    posz_ = 0;
    return true;
  } else if (event.key_code() == ui::VKEY_S) {
    negz_ = 0;
    return true;
  } else if (event.key_code() == ui::VKEY_A) {
    negx_ = 0;
    return true;
  } else if (event.key_code() == ui::VKEY_D) {
    posx_ = 0;
    return true;
  } else if (event.key_code() == ui::VKEY_F) {
    posy_ = 0;
    return true;
  } else if (event.key_code() == ui::VKEY_R) {
    negy_ = 0;
    return true;
  } else {
    return false;
  }
}

bool CameraController::OnMousePressed(const ui::MouseEvent& event) {
  location_ = event.location();
  if (event.IsLeftMouseButton() && event.GetClickCount() == 1) {
    origin_orient_ = camera_->holder().orientation();
    orientation_dragging_ = true;
    return true;
  } else {
    return false;
  }
}

bool CameraController::OnMouseDragged(const ui::MouseEvent& event) {
  if (orientation_dragging_) {
    RotateCamera(location_, event.location());
    return true;
  } else {
    return false;
  }
}

bool CameraController::OnMouseReleased(const ui::MouseEvent& event) {
  if (orientation_dragging_) {
    RotateCamera(location_, event.location());
    orientation_dragging_ = false;
    return true;
  } else {
    return false;
  }
}

void CameraController::RotateCamera(const gfx::Point& prev, 
                                    const gfx::Point& cur) {
  TransformHolder* holder = camera_->mutable_holder();
  holder->set_orientation(origin_orient_);
  Degree to_yaw = Degree(cur.x() - prev.x()) * 0.1;
  // holder->yaw(to_yaw);
  holder->rotate(azer::Vector3(0.0f, 1.0f, 0.0f), to_yaw);
  Degree to_pitch = Degree(cur.y() - prev.y()) * 0.1;
  holder->pitch(to_pitch);
}

void CameraController::Update(const azer::FrameArgs& args) {
  DCHECK(camera_);
  float unit = args.delta().InSecondsF() * 32.0f;
  TransformHolder* holder = camera_->mutable_holder();
  holder->strafe((posx_ - negx_) * unit);
  holder->walk((posz_ - negz_) * unit);
  holder->fly((posy_ - negy_)* unit);
  camera_->Update();
}

