#include "SkillLounge.h"
#include "ColShape.h"
#include "CollisionComponent.h"
#include "Components.h"
#include "GameWorld.h"
#include "HurtBoxComponent.h"
#include "InputComponent.h"
#include "LifeTimeComponent.h"
#include "MoveSystems.h"
#include "PosComponent.h"
#include "SkillComponent.h"
#include "StatusEffectComponent.h"
#include "TagComponent.h"
#include "WVecMath.h"
#include "steering.h"
#include "StatusEffectKnockback.h"
#include "StatusEffectHitstun.h"
#include "TagComponent.h"

bool
lounge_skill_hurtfunc(GameWorld& world,
                      unsigned int victim,
                      unsigned int attacker,
                      unsigned int)
{
  // knockback
  movement::interrupt(world, victim);
  auto dir = w_normalize(world.get<PosComponent>(victim).global_position -
                         world.get<PosComponent>(attacker).global_position);
  world.get<MoveComponent>(victim).velocity = dir * 1000.f;
  statuseffects::add_effect(world, victim, std::make_shared<Knockback>(.4f));
  statuseffects::add_effect(world, victim, std::make_shared<Hitstun>(0.1f));
  // TODO: damage
  return true;
}

bool
lounge_skill_on_hit(GameWorld& world,
                    unsigned int attacker,
                    unsigned int,
                    unsigned int)
{
  statuseffects::add_effect(world, attacker, std::make_shared<Hitstun>(0.1f));
  return true;
}

void
create_lounge_hurtbox(GameWorld& world,
                      unsigned int hurtbox,
                      unsigned int parent)
{
  bset comps;
  for (auto i :
       { CPosition, CColShape, CDynCol, CDebugDraw, CTag, CLifeTime }) {
    comps.set(i);
  }

  world.entities()[hurtbox] = comps;
  world.get<NameComponent>(hurtbox).name = "lounge_skill_hurtbox";

  // pos
  float radius = world.get<ColShapeComponent>(parent).shape->get_radius();
  auto& pc = world.get<PosComponent>(hurtbox);
  pc.parent = parent;
  pc.position = WVec(0, -radius * 0.3f);
  pc.rotation = 0;
  build_global_transform(world, hurtbox);
  // col shape
  auto& csc = world.get<ColShapeComponent>(hurtbox);
  csc.shape = std::make_shared<ColCapsule>(radius * 1.2f, radius * 0.7f);

  // tags
  auto& tc = world.get<TagComponent>(hurtbox);
  tc.tags.reset();
  tc.tags.set(static_cast<int>(Tags::Hurtbox));

  // dyn col
  set_dynamic_col(world.get<DynamicColComponent>(hurtbox),
                  DynColResponse::HurtBox);

  // lifetime
  auto& lc = world.get<LifeTimeComponent>(hurtbox);
  assert(world.get<MoveComponent>(parent).special_movestate != nullptr);
  lc.timer = world.get<MoveComponent>(parent).special_movestate->timer;

  // hurtbox
  auto& hb = world.get<HurtBoxComponent>(hurtbox);
  hb =
    HurtBoxComponent{ parent, {}, lounge_skill_hurtfunc, lounge_skill_on_hit };
}

void
LoungeAttackMove::enter(GameWorld& world, unsigned int entity)
{
  world.queue_create({ create_lounge_hurtbox, entity });
}

void
LoungeCastMove::accel(GameWorld& world, unsigned int entity)
{
  auto& pc = world.get<PosComponent>(entity);
  auto& ic = world.get<InputComponent>(entity);
  auto& mc = world.get<MoveComponent>(entity);

  rotate_to(ic.mouse.get(), mc.c_max_change_angle * 1.5f, pc);

  // cancel gravity
  mc.accel.y -= c_gravity;
  // slow caster down
  mc.accel -= w_normalize(mc.velocity) * w_magnitude(mc.velocity) * 0.9f;
}

void
LoungeAttackMove::accel(GameWorld& world, unsigned int entity)
{
  auto& pc = world.get<PosComponent>(entity);
  auto& mc = world.get<MoveComponent>(entity);
  auto& ic = world.get<InputComponent>(entity);

  float lounge_speed = 1200;
  float lounge_accel = 3500;
  float change_angle = mc.c_max_change_angle * 0.5f;

  auto vel = w_normalize(mc.velocity);
  // normal accel
  mc.accel = w_rotated(WVec(0, -1), pc.rotation * pc.direction) * lounge_accel;

  auto dir = w_normalize(ic.mouse.get() - pc.global_position);
  // when target before actor, steer towards
  if (dot(dir, w_rotated(WVec(0, -1), pc.rotation * pc.direction)) > 0) {
    auto angle =
      w_angle_to_vec(w_rotated(WVec(0, -1), pc.rotation * pc.direction), dir);
    rotate_angle(angle * pc.direction, change_angle, pc);

    // steer
    mc.accel = dir * lounge_accel;
  }

  mc.velocity = fmin(w_magnitude(mc.velocity), lounge_speed) * vel;
}

void
LoungeSkill::set_special(MoveComponent& mc)
{
  mc.special_movestate = std::make_unique<LoungeCastMove>();
}
