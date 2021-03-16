#include "DemoScene.h"

#include "scene/Scene.h"
#include "scene/Entity.h"

#include "scene/components/PlayerControllerComponent.h"
#include "scene/components/FollowMouseComponent.h"

static void Scene1(Scene* scene, PlayerControllerComponent* controller);
static void Scene2(Scene* scene, PlayerControllerComponent* controller);

PlayerControllerComponent* Demo::SetupDemoScene(Scene* scene, unsigned index)
{
    PlayerControllerComponent* controller = new PlayerControllerComponent(scene->GetCamera());

    switch (index)
    {
    case 1: Scene1(scene, controller); break;
    case 2: Scene2(scene, controller); break;
    default: Scene1(scene, controller); break;
    }

    return controller;
}

// Between -2.0 and 2.0
static float RandFloat(float min, float max)
{
    return (float)rand() / RAND_MAX * (max-min) + min;
}

static Vec2 RandPos(float min, float max)
{
    Vec2 randPos;
    randPos.x = RandFloat(min, max);
    randPos.y = RandFloat(min, max);
    return randPos;
}

static void Scene1(Scene* scene, PlayerControllerComponent* controller)
{
    // Entities
    Entity* bg = new Entity{ "res/bg.png" , "res/animals/elephant_normal.jpg" };
    bg->GetDrawFlags().background = true;
    bg->GetBackgroundProperties().textureScale = 5.0f;

    Entity* e1 = new Entity{ "res/animals/elephant.png", "res/animals/elephant_normal3.jpg" };
    e1->GetDrawFlags().occluder = true;
    e1->GetDrawFlags().foreground = true;
    e1->GetOcclusionProperties().shape = OccluderShape::Mesh;
    e1->GetOcclusionProperties().meshLod = 2;
    e1->AddComponent(controller);

    Entity* e2 = new Entity{ "res/animals/hippo.png" };
    e2->GetDrawFlags().emitter = true;
    e2->GetEmissionProperties().color = Vec3(0.0, 0.0, 1.0);
    e2->GetEmissionProperties().radius = 0.05f;

    Entity* e3 = new Entity{ "res/animals/giraffe.png" };
    e3->AddComponent(new FollowMouseComponent());
    e3->GetDrawFlags().emitter = true;
    e3->GetEmissionProperties().color = Vec3(1.0, 1.0, 0.0);
    e3->GetEmissionProperties().radius = 0.1f;

    Entity* e4 = new Entity{ "res/animals/elephant.png" , "res/animals/elephant_normal.jpg" };

    scene->AddEntity(bg);
    scene->AddEntity(e1);
    scene->AddEntity(e2);
    scene->AddEntity(e3);
    scene->AddEntity(e4);

    // Instances

    bg->Instance();

    EntityInstance* elephant = e1->Instance();
    elephant->ApplyRotation(3.1415f / 4.0f);

    EntityInstance* hippo = e2->Instance();
    hippo->ApplyScale(0.2f);
    hippo->SetPosition(Vec2(-0.3, 0.5));

    EntityInstance* giraffe = e3->Instance();
    giraffe->ApplyScale(0.3f);

    EntityInstance* elephant2 = e4->Instance();
    elephant2->ApplyScale(0.8f);
    elephant2->SetPosition(Vec2(0.8, 0.8));
}

static void Scene2(Scene* scene, PlayerControllerComponent* controller)
{
    // Entities
    const std::string res_path = "res/scene2/";

#define P(X) res_path + X

    Entity* bg = new Entity{P("bg_diffuse.png"), P("bg_normal.png") };
    bg->GetDrawFlags().background = true;
    bg->GetBackgroundProperties().textureScale = 2.0f;

    Entity* emitter = new Entity{ P("bg_diffuse.png"), P("bg_normal.png") };
    emitter->AddComponent(new FollowMouseComponent());
    emitter->AddComponent(controller);
    emitter->GetDrawFlags().emitter = true;
    emitter->GetEmissionProperties().color = Vec3(1.0, 1.0, 0.0);
    emitter->GetEmissionProperties().radius = 0.15f;

    Entity* tree1_up = new Entity{ P("tree1/tree1_up.png") };
    tree1_up->GetDrawFlags().foreground = true;

    Entity* tree1_down = new Entity{P("tree1/tree1_down.png")};
    tree1_down->GetDrawFlags().occluder = true;
    tree1_down->GetOcclusionProperties().shape = OccluderShape::Rect;

    Entity* tree2_up = new Entity{ P("tree2/tree2_up.png") };
    tree2_up->GetDrawFlags().foreground = true;

    Entity* tree2_down = new Entity{ P("tree2/tree2_down.png") };
    tree2_down->GetDrawFlags().occluder = true;
    tree2_down->GetOcclusionProperties().shape = OccluderShape::Rect;

    Entity* bush = new Entity{ P("bush_diffuse.png") };

#undef P

    scene->AddEntity(bg);
    scene->AddEntity(emitter);
    scene->AddEntity(tree1_up);
    scene->AddEntity(tree1_down);
    scene->AddEntity(tree2_up);
    scene->AddEntity(tree2_down);
    scene->AddEntity(bush);

    // Instances
    const Vec2 tree1Offset = Vec2(0.01, -0.2);
    const Vec2 tree2Offset = Vec2(0.0, -0.27);
    const unsigned numBushes = 100;
    const Vec2 numTrees = VEC2_ONE * 5.0f;

    bg->Instance();
    emitter->Instance()->ApplyScale(0.01f);

    for (unsigned i = 0; i < numBushes; i++)
    {
        bush->Instance()->SetPosition(RandPos(-2.0f,2.0f));
    }

    for (unsigned i = 0; i < (unsigned) numTrees.x; i++)
    {
        for (unsigned j = 0; j < (unsigned)numTrees.y; j++)
        {
            const float randVar = 0.1f;
            const Vec2 treeStep = 4.0f / numTrees;
            Vec2 treePos = Vec2(i, j) * treeStep - 2.0f;
            treePos += Vec2(RandFloat(-randVar, randVar), RandFloat(-randVar, randVar));
            float treeDecider = RandFloat(0.0f, 1.0f);

            if (treeDecider > 0.3f)
            {
                tree1_up->Instance()->SetPosition(treePos);
                tree1_down->Instance()->SetPosition(treePos + tree1Offset);
            }
            else
            {
                tree2_up->Instance()->SetPosition(treePos);
                tree2_down->Instance()->SetPosition(treePos + tree2Offset);
            }

        }
    }


}