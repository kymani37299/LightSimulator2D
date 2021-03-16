#include "DemoScene.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

#include "scene/components/PlayerControllerComponent.h"
#include "scene/components/FollowMouseComponent.h"

static void Scene1(Scene* scene, PlayerControllerComponent* controller);

PlayerControllerComponent* Demo::SetupDemoScene(Scene* scene, unsigned index)
{
    PlayerControllerComponent* controller = new PlayerControllerComponent(scene->GetCamera());

    switch (index)
    {
    case 1: Scene1(scene, controller); break;
    default: Scene1(scene, controller); break;
    }

    return controller;
}

static void Scene1(Scene* scene, PlayerControllerComponent* controller)
{
    Entity* bg = new Entity{ "res/bg.png" , "res/animals/elephant_normal.jpg" };
    bg->GetDrawFlags().background = true;
    bg->GetBackgroundProperties().textureScale = 5.0f;

    Entity* e1 = new Entity{ "res/animals/elephant.png", "res/animals/elephant_normal3.jpg" };
    e1->GetDrawFlags().occluder = true;
    e1->GetOcclusionProperties().shape = OccluderShape::Mesh;
    e1->GetOcclusionProperties().meshLod = 2;
    e1->m_Transform.rotation = 3.1415f / 4.0f;
    e1->AddComponent(controller);

    Entity* e2 = new Entity{ "res/animals/hippo.png" };
    e2->GetDrawFlags().emitter = true;
    e2->GetEmissionProperties().color = Vec3(0.0, 0.0, 1.0);
    e2->GetEmissionProperties().radius = 0.05f;
    e2->m_Transform.scale *= 0.2;
    e2->m_Transform.position = Vec2(-0.3, 0.5);

    Entity* e3 = new Entity{ "res/animals/giraffe.png" };
    e3->AddComponent(new FollowMouseComponent());
    e3->m_Transform.scale *= 0.3;
    e3->GetDrawFlags().emitter = true;
    e3->GetEmissionProperties().color = Vec3(1.0, 1.0, 0.0);
    e3->GetEmissionProperties().radius = 0.1f;

    Entity* e4 = new Entity{ "res/animals/elephant.png" , "res/animals/elephant_normal.jpg" };
    e4->m_Transform.scale *= 0.8;
    e4->m_Transform.position = Vec2(0.8, 0.8);

    scene->AddEntity(bg);
    scene->AddEntity(e1);
    scene->AddEntity(e2);
    scene->AddEntity(e3);
    scene->AddEntity(e4);
}