/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include <toolbox_stdafx.h>
#include "SkyBox.h"
#include "NewtonDemos.h"
#include "TargaToOpenGl.h"
#include "DemoMesh.h"
#include "DemoEntityManager.h"
#include "DemoCamera.h"
#include "PhysicsUtils.h"
#include "HeightFieldPrimitive.h"
#include "DebugDisplay.h"

#if 0
struct VehicleParameters
{
	dFloat MASS;
	dFloat TIRE_MASS;
	dFloat STEER_ANGLE;
	dFloat BRAKE_TORQUE;
	dFloat COM_Y_OFFSET;
	dFloat TIRE_TOP_SPEED_KMH;

	dFloat IDLE_TORQUE;
	dFloat IDLE_TORQUE_RPM;

	dFloat PEAK_TORQUE;
	dFloat PEAK_TORQUE_RPM;

	dFloat PEAK_HP;
	dFloat PEAK_HP_RPM;

	dFloat REDLINE_TORQUE;
	dFloat REDLINE_TORQUE_RPM;

	dFloat GEAR_1;
	dFloat GEAR_2;
	dFloat GEAR_3;
	dFloat REVERSE_GEAR;

	dFloat SUSPENSION_LENGTH;
	dFloat SUSPENSION_SPRING;
	dFloat SUSPENSION_DAMPER;
	dFloat LATERAL_STIFFNESS;
	dFloat LONGITUDINAL_STIFFNESS;
	dFloat ALIGNING_MOMENT_TRAIL;

	dMatrix m_tireAligment;
};

static VehicleParameters heavyTruck = 
{
	5000.0f,	// MASS
	 100.0f,	// TIRE_MASS
	  25.0f,	// STEER_ANGLE
	10000.0f,	// BRAKE_TORQUE
	  -0.6f,	// COM_Y_OFFSET
	 60.0f,		// TIRE_TOP_SPEED_KMH
	3500.0f,	// IDLE_TORQUE
	500.0f,		// IDLE_TORQUE_RPM
	1200.0f,	// PEAK_TORQUE
	3000.0f,	// PEAK_TORQUE_RPM
	 800.0f,	// PEAK_HP
	4000.0f,	// PEAK_HP_RPM
	 300.0f,	// REDLINE_TORQUE
	4500.0f,	// REDLINE_TORQUE_RPM
		2.5f,	// GEAR_1
		2.0f,	// GEAR_2
		1.5f,	// GEAR_3
		2.9f,	// REVERSE_GEAR
	   1.2f,	// SUSPENSION_LENGTH
	2000.0f,	// SUSPENSION_SPRING
	 200.0f,	// SUSPENSION_DAMPER
	  20.0f,	// LATERAL_STIFFNESS
  100000.0f,	// LONGITUDINAL_STIFFNESS
	   1.5f,	// ALIGNING_MOMENT_TRAIL
	   dGetIdentityMatrix(),
};

static VehicleParameters lightTruck = 
{
	2000.0f,	// MASS
	100.0f,		// TIRE_MASS
	25.0f,		// STEER_ANGLE
	3000.0f,	// BRAKE_TORQUE
	-0.6f,		// COM_Y_OFFSET
	72.0f,		// TIRE_TOP_SPEED_KMH
	1200.0f,	// IDLE_TORQUE
	500.0f,		// IDLE_TORQUE_RPM
	 500.0f,	// PEAK_TORQUE
	3000.0f,	// PEAK_TORQUE_RPM
	 300.0f,	// PEAK_HP
	4000.0f,	// PEAK_HP_RPM
	  60.0f,	// REDLINE_TORQUE
	4500.0f,	// REDLINE_TORQUE_RPM
	2.5f,		// GEAR_1
	2.0f,		// GEAR_2
	1.5f,		// GEAR_3
	2.9f,		// REVERSE_GEAR
	0.6f,		// SUSPENSION_LENGTH
	2000.0f,	// SUSPENSION_SPRING
	100.0f,		// SUSPENSION_DAMPER
	20.0f,		// LATERAL_STIFFNESS
	20000.0f,	// LONGITUDINAL_STIFFNESS
	1.5f,		// ALIGNING_MOMENT_TRAIL
	dRollMatrix(3.141592f * 90.0f / 180.0f)
};



static VehicleParameters m1a1Param = 
{
	5000.0f,	// MASS
	100.0f,		// TIRE_MASS
	25.0f,		// STEER_ANGLE
	20000.0f,	// BRAKE_TORQUE
	-0.6f,		// COM_Y_OFFSET
	60.0f,		// TIRE_TOP_SPEED_KMH
	3500.0f,	// IDLE_TORQUE
	500.0f,		// IDLE_TORQUE_RPM
	1200.0f,	// PEAK_TORQUE
	3000.0f,	// PEAK_TORQUE_RPM
	800.0f,		// PEAK_HP
	4000.0f,	// PEAK_HP_RPM
	300.0f,		// REDLINE_TORQUE
	4500.0f,	// REDLINE_TORQUE_RPM
	2.5f,		// GEAR_1
	2.0f,		// GEAR_2
	1.5f,		// GEAR_3
	2.9f,		// REVERSE_GEAR
	1.2f,		// SUSPENSION_LENGTH
	350.0f,		// SUSPENSION_SPRING
	20.0f,		// SUSPENSION_DAMPER
	60.0f,		// LATERAL_STIFFNESS
	10000.0f,	// LONGITUDINAL_STIFFNESS
	1.5f,		// ALIGNING_MOMENT_TRAIL
	dRollMatrix(3.141592f * 90.0f / 180.0f)
};


#define HEAVY_VEHICLE_THIRD_PERSON_VIEW_HIGHT		2.0f
#define HEAVY_VEHICLE_THIRD_PERSON_VIEW_DIST		15.0f
#define HEAVY_VEHICLE_THIRD_PERSON_VIEW_FILTER		0.125f


class HeavyVehicleEntity: public DemoEntity
{
	public: 
	class TrackSystem
	{
		public:
		class ConstantSpeedKnotInterpolant
		{
			public:
			dFloat m_u;
			dFloat m_dist;
		};

		TrackSystem()
			:m_bezierEntity(NULL)
			,m_bezierMesh(NULL)
			,m_intepolants(NULL)
			,m_linksEntity(NULL)
			,m_controlPointsOffset(NULL)
			,m_controlPointBindIndex(NULL)
		{
		}

		~TrackSystem()
		{
			if (m_intepolants) {
				delete[] m_linksEntity;
				delete[] m_intepolants;
			}
			if (m_controlPointsOffset) {
				delete[] m_controlPointsOffset;
				delete[] m_controlPointBindIndex;
			}
		}


		dFloat GetTireRadius (DemoEntity* const tire) const
		{
			for (CustomVehicleControllerBodyStateTire* tireNode = m_vehicle->m_controller->GetFirstTire (); tireNode; tireNode = m_vehicle->m_controller->GetNextTire(tireNode)) {
				if (tireNode->GetUserData() == tire) {
					return tireNode->GetTireRadio();
				}
			}
			dAssert (0);
			return 0.0f;
		}

		void CalculaterBinding()
		{
			m_controlPointCount = m_bezierMesh->m_curve.GetControlPointCount();
			m_controlPointsOffset = new dVector[m_controlPointCount];
			m_controlPointBindIndex = new int[m_controlPointCount];

			DemoEntity* const tire = (DemoEntity*) m_tire->GetUserData();
			dString name (tire->GetName());
			name.Replace(name.Find ("0"), 1, "", 0);

			dFloat radius[8];  
			dMatrix tireMatrix[8];  
			for (int i = 0; i < 8; i ++) {
				dString name1 (name + i);
				dAssert (m_vehicle->Find(name1.GetStr()));
				DemoEntity* const tireEnt = m_vehicle->Find(name1.GetStr());
				m_bindingTires[i] = tireEnt;
				radius[i] = GetTireRadius(tireEnt);
				tireMatrix[i] = tireEnt->GetMeshMatrix() * tireEnt->GetCurrentMatrix();
			}

			for (int i = 0; i < m_controlPointCount; i ++) {
				m_controlPointBindIndex[i] = -1;
				dBigVector q (m_bezierMesh->m_curve.GetControlPoint(i));
				m_controlPointsOffset[i] = m_shapeMatrix.TransformVector(dVector (q.m_x, q.m_y, q.m_z, q.m_w));
			}

			for (int i = 0; i < m_controlPointCount; i ++) {
				int index = -1;
				dFloat dist2 = 1.0e10f;

				const dVector& p0 = m_controlPointsOffset[i];
				for (int j = 0; j < 8; j ++) {
					dVector dist (p0 - tireMatrix[j].m_posit);
					dFloat mag2 = dist % dist;
					if (mag2 < dist2) {
						dist2 = mag2;
						index = j;
					}
				}
				dAssert (index != -1);
				dFloat r2 = (radius[index] + 0.2f) * (radius[index] + 0.2f);
				if (dist2 < r2) {
					m_controlPointBindIndex[i] = index;
					m_controlPointsOffset[i] -= tireMatrix[index].m_posit;
				}
			}
		}

		void CalculaterUniformSpaceSamples()
		{
			DemoEntity* const threadMesh = m_vehicle->Find ("threadShoe");
			dAssert (threadMesh);
			dFloat threadSize = 0.29f;
			m_linksCount = int (m_length / threadSize) + 1;
			threadSize = m_length / m_linksCount;
			
			m_linksEntity = new DemoEntity*[m_linksCount];
			for (int i = 0; i < m_linksCount; i ++) {
				DemoEntity* const threadPart = new DemoEntity (dGetIdentityMatrix(), m_vehicle);
				threadPart->SetMesh(threadMesh->GetMesh(), dGetIdentityMatrix());
				m_linksEntity[i] = threadPart;
			}


			// create the uniform speed knot interpolation table
			ConstantSpeedKnotInterpolant steps[1024];
			steps[0].m_u = 0.0f;
			steps[0].m_dist = 0.0f;

			int count = 0;
			steps[count].m_u = 0.0f;
			steps[count].m_dist = 0.0f;
			count ++;
			int samplingRate = m_linksCount * 10;
			dFloat distAcc = 0.0f;

			threadSize *= 0.25f;
			dFloat stepAcc = threadSize;
			dBigVector q (m_bezierMesh->m_curve.CurvePoint(0.0f));
			dVector p0 (dVector (q.m_x, q.m_y, q.m_z, q.m_w));
			for (int i = 1; i < samplingRate + 15; i ++) {
				dFloat u = dFloat (i) / samplingRate;
				dBigVector q (m_bezierMesh->m_curve.CurvePoint(dMod (u, 1.0f)));
				dVector p1 (dVector (q.m_x, q.m_y, q.m_z, q.m_w));
				dVector err (p1 - p0);
				dFloat errMag = dSqrt (err % err);
				distAcc += errMag;
				if (distAcc > stepAcc) {
					stepAcc += threadSize;
					steps[count].m_u = u;
					steps[count].m_dist = distAcc;
					count ++;
					dAssert (count < int (sizeof (steps) / sizeof (steps[0])));
				}
				p0 = p1;
			}

			m_interpolantsCount = count;
			m_intepolants = new ConstantSpeedKnotInterpolant[count];
			for (int i = 0; i < count; i ++) {
				m_intepolants[i] = steps[i];
			}

			m_aligmentMatrix = dRollMatrix(3.141592f * 90.0f / 180.0f) * dPitchMatrix(3.141592f * 180.0f / 180.0f);
			m_shapeMatrix = m_bezierEntity->GetMeshMatrix() * m_bezierEntity->GetCurrentMatrix();
		}

		void Init (HeavyVehicleEntity* const me, const char* const name, CustomVehicleControllerBodyStateTire* const tire)
		{
			m_vehicle = me;
			m_tire = tire;
			m_parameter = 0.0f;
			m_bezierEntity = me->Find (name);
			dAssert (m_bezierEntity);
			m_bezierMesh = (DemoBezierCurve*) m_bezierEntity->GetMesh();
			dAssert (m_bezierMesh->IsType(DemoBezierCurve::GetRttiType()));

			m_length = m_bezierMesh->m_curve.CalculateLength (0.01f);

			m_bezierEntity = me->Find (name);
			CalculaterUniformSpaceSamples();
			CalculaterBinding();

			NewtonWorld* const world = NewtonBodyGetWorld (m_tire->GetController()->GetBody());
			AnimateThread (*((DemoEntityManager*)NewtonWorldGetUserData(world)), 0.0f);
			AnimateThread (*((DemoEntityManager*)NewtonWorldGetUserData(world)), 0.0f);
		}

		dFloat CalculateKnotParam (dFloat t) const
		{
			int low = 0;
			int high = m_interpolantsCount - 1;

			while ((high - low) >= 4) {
				int mid = (low + high) >> 1;
				if (t > m_intepolants[mid].m_dist) {
					low = mid;
				} else {
					high = mid;
				}
			}

			dAssert (m_intepolants[low].m_dist <= t);
			for (int i = low; i < m_interpolantsCount; i ++) {
				if (m_intepolants[i + 1].m_dist >= t) {
					low = i;
					break;
				}
			}

			dFloat u0 = m_intepolants[low].m_u;
			dFloat h0 = m_intepolants[low].m_dist;
			dFloat du = m_intepolants[low + 1].m_u - u0;
			dFloat dh = m_intepolants[low + 1].m_dist - h0;
			return dMod (u0 + du * (t - h0) / dh, 1.0f);
		}

		void InterpolateMatrix (DemoEntityManager& world, dFloat param)
		{
			for (int i = 0; i < m_linksCount; i ++) {
				DemoEntity* const threadPart = m_linksEntity[i];
				threadPart->InterpolateMatrix (world, param);
			}
		}

		void AnimateThread (DemoEntityManager& world, dFloat timestep) 
		{
			dVector matrixPalette[8];
			for (int i = 0; i < 8; i ++) {
				matrixPalette[i] = (m_bindingTires[i]->GetMeshMatrix() * m_bindingTires[i]->GetCurrentMatrix()).m_posit;
			}

			for (int i = 0; i < m_controlPointCount; i ++) {
				if (m_controlPointBindIndex[i] != -1) {
					int index = m_controlPointBindIndex[i];
					dVector p (m_controlPointsOffset[i] + matrixPalette[index]);
					m_bezierMesh->m_curve.SetControlPoint (i, m_shapeMatrix.UntransformVector(p));
				}
			}

			dFloat radio = m_tire->GetTireRadio();
			dFloat omega = m_tire->GetTireRotationSpeed();

			dFloat step = omega * radio * timestep;

			m_parameter = dMod (m_parameter + step, m_length);
			if (m_parameter < 0.0f) {
				m_parameter += m_length;
			}
			dAssert (m_parameter >= 0.0f);
			dAssert (m_parameter <= m_length);

			dMatrix matrix (dGetIdentityMatrix());
			dFloat size = m_length / m_linksCount;
			dFloat u0 = CalculateKnotParam (dMod (m_parameter + (m_linksCount - 1) * size, m_length));
			dBigVector q (m_bezierMesh->m_curve.CurvePoint(u0));
			dVector q0 (dVector (q.m_x, q.m_y, q.m_z, q.m_w));
			for (int i = 0; i < m_linksCount; i ++) {
				DemoEntity* const threadPart = m_linksEntity[i];
				dFloat s = dMod (m_parameter + i * size, m_length);
				dFloat u1 = CalculateKnotParam (s);
				dBigVector q (m_bezierMesh->m_curve.CurvePoint(u1));
				dVector q1 (dVector (q.m_x, q.m_y, q.m_z, q.m_w));
				dVector dir (q1 - q0);

				dir = dir.Scale (1.0f / dSqrt (dir % dir));
				matrix.m_front = dVector (dir.m_z, -dir.m_y, 0.0f, 0.0f);
				matrix.m_up = dVector (dir.m_y, dir.m_z, 0.0f, 0.0f);
				matrix.m_right = dVector (0.0f, 0.0f, 1.0f, 0.0f);
				matrix = m_aligmentMatrix * matrix;
				matrix.m_posit = m_shapeMatrix.TransformVector (q0);
				threadPart->SetMatrix (world, dQuaternion(matrix), matrix.m_posit);
				q0 = q1;
				u0 = u1;
			}
		}

		dMatrix m_shapeMatrix;
		dMatrix m_aligmentMatrix;
		
		HeavyVehicleEntity* m_vehicle;
		DemoEntity* m_bezierEntity;
		DemoBezierCurve* m_bezierMesh;
		CustomVehicleControllerBodyStateTire* m_tire;
		ConstantSpeedKnotInterpolant* m_intepolants;
		DemoEntity* m_bindingTires[8];
		DemoEntity** m_linksEntity;
		dVector* m_controlPointsOffset;
		int* m_controlPointBindIndex;

		dFloat m_length;
		dFloat m_parameter;
		int m_linksCount;
		int m_controlPointCount;
		int m_interpolantsCount;
	};

	class TireAligmentTransform: public UserData
	{
		public: 
		TireAligmentTransform (const dMatrix& matrix)
			:UserData()
			,m_matrix (matrix)
		{
		}

		virtual void OnRender (dFloat timestep) const{};
		virtual void OnInterpolateMatrix (DemoEntityManager& world, dFloat param) const{};

		dMatrix m_matrix;
	};


	HeavyVehicleEntity (DemoEntityManager* const scene, CustomVehicleControllerManager* const manager, const dMatrix& location, const char* const filename, const VehicleParameters& parameters)
		:DemoEntity (dGetIdentityMatrix(), NULL)
		,m_controller(NULL)
		,m_helpKey (true)
		,m_gearUpKey (false)
		,m_gearDownKey (false)
		,m_reverseGear (false)
		,m_engineKeySwitch(false)
		,m_automaticTransmission(true)
		,m_engineKeySwitchCounter(0)
		,m_engineOldKeyState(false)
		,m_engineRPMOn(false)
	{
		// add this entity to the scene for rendering
		scene->Append(this);

		// load the visual mesh 
		LoadNGD_mesh (filename, scene->GetNewton());

		// place entity in the world
		ResetMatrix (*scene, location);

		NewtonWorld* const world = scene->GetNewton();

		// create the car rigid body
		//NewtonBody* const body = CreateChassisBody (carEntity, world, materialID, params);
		NewtonCollision* const chassisCollision = CreateChassisCollision (world);

		// create the vehicle controller
		dMatrix chassisMatrix;
		chassisMatrix.m_front = dVector (1.0f, 0.0f, 0.0f, 0.0f);			// this is the vehicle direction of travel
		chassisMatrix.m_up	  = dVector (0.0f, 1.0f, 0.0f, 0.0f);			// this is the downward vehicle direction
		chassisMatrix.m_right = chassisMatrix.m_front * chassisMatrix.m_up;	// this is in the side vehicle direction (the plane of the wheels)
		chassisMatrix.m_posit = dVector (0.0f, 0.0f, 0.0f, 1.0f);

		// create a default vehicle 
		m_controller = manager->CreateVehicle (chassisCollision, chassisMatrix, parameters.MASS, dVector (0.0f, DEMO_GRAVITY, 0.0f, 0.0f));

		// get body from player
		NewtonBody* const body = m_controller->GetBody();

		// set the user data
		NewtonBodySetUserData(body, this);

		// set the transform callback
		NewtonBodySetTransformCallback (body, DemoEntity::TransformCallback);

		// set the standard force and torque call back
		NewtonBodySetForceAndTorqueCallback(body, PhysicsApplyGravityForce);

		// set the player matrix 
		NewtonBodySetMatrix(body, &location[0][0]);

		// destroy the collision helper shape 
		NewtonDestroyCollision(chassisCollision);

		for (int i = 0; i < int ((sizeof (m_gearMap) / sizeof (m_gearMap[0]))); i ++) {
			m_gearMap[i] = i;
		}
		m_gearMap[0] = 1;
		m_gearMap[1] = 0;
	}

	~HeavyVehicleEntity ()
	{
		((CustomVehicleControllerManager*)m_controller->GetManager())->DestroyController(m_controller);
	}

	NewtonCollision* CreateChassisCollision (NewtonWorld* const world) const
	{
		DemoEntity* const chassis = dHierarchy<DemoEntity>::Find("chassis");
		dAssert (chassis);

		DemoMesh* const mesh = (DemoMesh*)chassis->GetMesh();
		dAssert (mesh->IsType(DemoMesh::GetRttiType()));
		//dAssert (chassis->GetMeshMatrix().TestIdentity());
		const dMatrix& meshMatrix = chassis->GetMeshMatrix();

		dVector* const temp = new dVector[mesh->m_vertexCount];
		meshMatrix.TransformTriplex (&temp[0].m_x, sizeof (dVector), mesh->m_vertex, 3 * sizeof (dFloat), mesh->m_vertexCount);
		NewtonCollision* const shape = NewtonCreateConvexHull (world, mesh->m_vertexCount, &temp[0].m_x, sizeof (dVector), 0.001f, 0, NULL);
		delete[] temp;
		return shape;
	}

	// interpolate all skeleton transform 
	virtual void InterpolateMatrix (DemoEntityManager& world, dFloat param)
	{
		DemoEntity::InterpolateMatrix (world, param);
		if (m_controller){
			for (CustomVehicleControllerBodyStateTire* tire = m_controller->GetFirstTire(); tire; tire = m_controller->GetNextTire(tire)) {
				DemoEntity* const tirePart = (DemoEntity*) tire->GetUserData();
				tirePart->InterpolateMatrix (world, param);
			}
		}

		if (m_leftTrack.m_bezierEntity) {
			m_leftTrack.InterpolateMatrix (world, param);
			m_rightTrack.InterpolateMatrix  (world, param);
		}
	}

	void CalculateTireDimensions (const char* const tireName, dFloat& width, dFloat& radius) const
	{
		NewtonBody* const body = m_controller->GetBody();
		NewtonWorld* const world = NewtonBodyGetWorld(body);

		// find the the tire visual mesh 
		DemoEntity* const entity = (DemoEntity*) NewtonBodyGetUserData(body);
		DemoEntity* const tirePart = entity->Find (tireName);
		dAssert (tirePart);

		// make a convex hull collision shape to assist in calculation of the tire shape size
		DemoMesh* const tireMesh = (DemoMesh*)tirePart->GetMesh();
		dAssert (tireMesh->IsType(DemoMesh::GetRttiType()));
		dMatrix meshMatrix (tirePart->GetMeshMatrix());
		dVector* const temp = new dVector [tireMesh->m_vertexCount];
		meshMatrix.TransformTriplex (&temp[0].m_x, sizeof (dVector), tireMesh->m_vertex, 3 * sizeof (dFloat), tireMesh->m_vertexCount);
		NewtonCollision* const collision = NewtonCreateConvexHull(world, tireMesh->m_vertexCount, &temp[0].m_x, sizeof (dVector), 0, 0, NULL);
		delete[] temp;

		// get the location of this tire relative to the car chassis
		dMatrix tireMatrix (tirePart->CalculateGlobalMatrix(entity));

		// find the support points that can be used to define the with and high of the tire collision mesh
		dVector extremePoint;
		dVector upDir (tireMatrix.UnrotateVector(dVector (0.0f, 1.0f, 0.0f, 0.0f)));
		NewtonCollisionSupportVertex (collision, &upDir[0], &extremePoint[0]);
		radius = dAbs (upDir % extremePoint);

		dVector widthDir (tireMatrix.UnrotateVector(dVector (0.0f, 0.0f, 1.0f, 0.0f)));
		NewtonCollisionSupportVertex (collision, &widthDir[0], &extremePoint[0]);
		width = widthDir % extremePoint;

		widthDir = widthDir.Scale (-1.0f);
		NewtonCollisionSupportVertex (collision, &widthDir[0], &extremePoint[0]);
		width += widthDir % extremePoint;

		// destroy the auxiliary collision
		NewtonDestroyCollision (collision);	
	}


	CustomVehicleControllerBodyStateTire* AddTire (const char* const tireName, const dVector& offset, dFloat width, dFloat radius, dFloat mass, dFloat suspensionLength, dFloat suspensionSpring, dFloat suspensionDamper, dFloat lateralStiffness, dFloat longitudinalStiffness, dFloat aligningMomentTrail, const dMatrix& tireAligmentMatrix) 
	{
		NewtonBody* const body = m_controller->GetBody();
		DemoEntity* const entity = (DemoEntity*) NewtonBodyGetUserData(body);
		DemoEntity* const tirePart = entity->Find (tireName);

		// add this tire, get local position and rise it by the suspension length 
		dMatrix tireMatrix (tirePart->CalculateGlobalMatrix(entity));

		// add the offset location
		tireMatrix.m_posit += offset;

		//lower the tire base position by some distance
		tireMatrix.m_posit.m_y -= suspensionLength * 0.25f;

		// add and alignment matrix,to match visual mesh to physics collision shape
		dMatrix aligmentMatrix (dGetIdentityMatrix());
		if (tireMatrix[0][2] < 0.0f) {
			aligmentMatrix = dYawMatrix(3.141592f);
		}

		TireAligmentTransform* const ligmentMatrix = new TireAligmentTransform (tireAligmentMatrix * aligmentMatrix);
		tirePart->SetUserData(ligmentMatrix);

		// add the tire to the vehicle
		CustomVehicleControllerBodyStateTire::TireCreationInfo tireInfo;
		tireInfo.m_location = tireMatrix.m_posit;
		tireInfo.m_mass = mass;
		tireInfo.m_radio = radius;
		tireInfo.m_width = width;
		tireInfo.m_dampingRatio = suspensionDamper;
		tireInfo.m_springStrength = suspensionSpring;
		tireInfo.m_suspesionlenght = suspensionLength;
		tireInfo.m_lateralStiffness = lateralStiffness;
		tireInfo.m_longitudialStiffness = longitudinalStiffness;
		tireInfo.m_aligningMomentTrail =  aligningMomentTrail;
		tireInfo.m_userData = tirePart;

		return m_controller->AddTire (tireInfo);
	}

	void UpdateTireTransforms(dFloat timestep)
	{
		NewtonBody* const body = m_controller->GetBody();
		//NewtonWorld* const world = NewtonBodyGetWorld(body);
		DemoEntityManager* const scene = (DemoEntityManager*) NewtonWorldGetUserData(NewtonBodyGetWorld(body));
		
#if 0
		// this is the general way for getting the tire matrices
		dMatrix rootMatrixInv (GetNextMatrix().Inverse());
		for (CustomVehicleControllerBodyStateTire* tire = m_controller->GetFirstTire(); tire; tire = m_controller->GetNextTire(tire)) {
			DemoEntity* const tirePart = (DemoEntity*) tire->GetUserData();
			TireAligmentTransform* const aligmentMatrix = (TireAligmentTransform*)tirePart->GetUserData();
			dMatrix matrix (aligmentMatrix->m_matrix * tire->CalculateGlobalMatrix() * rootMatrixInv);
			dQuaternion rot (matrix);
			tirePart->SetMatrix(*scene, rot, matrix.m_posit);
		}
#else
		// this saves some calculation since it get the tire local to the chassis
		for (CustomVehicleControllerBodyStateTire* tire = m_controller->GetFirstTire(); tire; tire = m_controller->GetNextTire(tire)) {
			DemoEntity* const tirePart = (DemoEntity*) tire->GetUserData();
			TireAligmentTransform* const aligmentMatrix = (TireAligmentTransform*)tirePart->GetUserData();
			dMatrix matrix (aligmentMatrix->m_matrix * tire->CalculateLocalMatrix());
			dQuaternion rot (matrix);
			tirePart->SetMatrix(*scene, rot, matrix.m_posit);
		}
#endif

		// update the thread location
		if (m_leftTrack.m_bezierEntity) {
			m_leftTrack.AnimateThread (*scene, timestep);
			m_rightTrack.AnimateThread (*scene, timestep);
		}
	}

	// this function is an example of how to make a high performance super car
	void BuildAllWheelDriveVehicle (const VehicleParameters& parameters)
	{
		// Muscle cars have the front engine, we need to shift the center of mass to the front to represent that
		m_controller->SetCenterOfGravity (dVector (0.0f, parameters.COM_Y_OFFSET, 0.0f, 0.0f)); 


		// a car may have different size front an rear tire, therefore we do this separate for front and rear tires
		dFloat width;
		dFloat radius;
		CalculateTireDimensions ("ltire_0", width, radius);

		dVector offset (0.0f, 0.35f, 0.0f, 0.0f);

		// add left tires
		CustomVehicleControllerBodyStateTire* leftTire[4]; 
		leftTire[0] = AddTire ("ltire_0", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		leftTire[1] = AddTire ("ltire_1", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		leftTire[2] = AddTire ("ltire_2", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		leftTire[3] = AddTire ("ltire_3", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);

		// add right tires
		CustomVehicleControllerBodyStateTire* rightTire[4]; 
		rightTire[0] = AddTire ("rtire_0", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		rightTire[1] = AddTire ("rtire_1", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		rightTire[2] = AddTire ("rtire_2", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		rightTire[3] = AddTire ("rtire_3", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);

		// add a steering Wheel
		CustomVehicleControllerComponentSteering* const steering = new CustomVehicleControllerComponentSteering (m_controller, parameters.STEER_ANGLE * 3.141592f / 180.0f);
		for (int i = 0; i < 2; i ++) {
			steering->AddSteeringTire (leftTire[i]);
			steering->AddSteeringTire (rightTire[i]);
		}
		//steering->CalculateAkermanParameters (leftRearTire, rightRearTire, leftFrontTire, rightFrontTire);
		m_controller->SetSteering(steering);

		// add vehicle brakes
		CustomVehicleControllerComponentBrake* const brakes = new CustomVehicleControllerComponentBrake (m_controller, parameters.BRAKE_TORQUE);
		for (int i = 0; i < 4; i ++) {
			brakes->AddBrakeTire (leftTire[i]);
			brakes->AddBrakeTire (rightTire[i]);
		}
		m_controller->SetBrakes(brakes);


		// add an engine
		// make the differential
		CustomVehicleControllerComponentEngine::dSingleAxelDifferential* axels[4];
		for (int i = 0; i < 4; i ++) {
			axels[i] = new CustomVehicleControllerComponentEngine::dSingleAxelDifferential (m_controller, leftTire[i], rightTire[i]);
		}
		CustomVehicleControllerComponentEngine::dMultiAxelDifferential* const differencial = new CustomVehicleControllerComponentEngine::dMultiAxelDifferential (m_controller, 4, axels);

		// make the gear Box
		dFloat fowardSpeedGearsBoxRatios[] = {parameters.GEAR_1, parameters.GEAR_1, parameters.GEAR_3};
		CustomVehicleControllerComponentEngine::dGearBox* const gearBox = new CustomVehicleControllerComponentEngine::dGearBox(m_controller, parameters.REVERSE_GEAR, sizeof (fowardSpeedGearsBoxRatios) / sizeof (fowardSpeedGearsBoxRatios[0]), fowardSpeedGearsBoxRatios); 
		CustomVehicleControllerComponentEngine* const engine = new CustomVehicleControllerComponentEngine (m_controller, gearBox, differencial);

		// the the default transmission type
		engine->SetTransmissionMode(m_automaticTransmission.GetPushButtonState());

		m_controller->SetEngine(engine);


		dFloat viperIdleRPM = parameters.IDLE_TORQUE_RPM;
		dFloat viperIdleTorquePoundPerFoot = parameters.IDLE_TORQUE;

		dFloat viperPeakTorqueRPM = parameters.PEAK_TORQUE_RPM;
		dFloat viperPeakTorquePoundPerFoot = parameters.PEAK_TORQUE;

		dFloat viperPeakHorsePowerRPM = parameters.PEAK_HP_RPM;
		dFloat viperPeakHorsePower = parameters.PEAK_HP;

		dFloat viperRedLineRPM = parameters.REDLINE_TORQUE_RPM;
		dFloat viperRedLineTorquePoundPerFoot = parameters.REDLINE_TORQUE;

		dFloat vehicleTopSpeedKPH = parameters.TIRE_TOP_SPEED_KMH;
		engine->InitEngineTorqueCurve (vehicleTopSpeedKPH, viperIdleTorquePoundPerFoot, viperIdleRPM, viperPeakTorquePoundPerFoot, viperPeakTorqueRPM, viperPeakHorsePower, viperPeakHorsePowerRPM, viperRedLineTorquePoundPerFoot, viperRedLineRPM);

		// do not forget to call finalize after all components are added or after any change is made to the vehicle
		m_controller->Finalize();
	}

	void BuildLightTruckVehicle (const VehicleParameters& parameters)
	{
		// Muscle cars have the front engine, we need to shift the center of mass to the front to represent that
		m_controller->SetCenterOfGravity (dVector (0.0f, parameters.COM_Y_OFFSET, 0.0f, 0.0f)); 
		
		// a car may have different size front an rear tire, therefore we do this separate for front and rear tires
		dFloat width;
		dFloat radius;
		dVector offset (0.0f, 0.15f, 0.0f, 0.0f);
		CalculateTireDimensions ("fl_tire", width, radius);

		// add left tires
		CustomVehicleControllerBodyStateTire* leftTire[2]; 
		leftTire[0] = AddTire ("fl_tire", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		leftTire[1] = AddTire ("rl_tire", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);

		// add right tires
		CustomVehicleControllerBodyStateTire* rightTire[2];
		CalculateTireDimensions ("rl_tire", width, radius);
		rightTire[0] = AddTire ("fr_tire", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		rightTire[1] = AddTire ("rr_tire", offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);

		// add a steering Wheel
		CustomVehicleControllerComponentSteering* const steering = new CustomVehicleControllerComponentSteering (m_controller, parameters.STEER_ANGLE * 3.141592f / 180.0f);
		steering->AddSteeringTire (leftTire[0]);
		steering->AddSteeringTire (rightTire[0]);
		//steering->CalculateAkermanParameters (leftRearTire, rightRearTire, leftFrontTire, rightFrontTire);
		m_controller->SetSteering(steering);

		// add vehicle brakes
		CustomVehicleControllerComponentBrake* const brakes = new CustomVehicleControllerComponentBrake (m_controller, parameters.BRAKE_TORQUE);
		for (int i = 0; i < 2; i ++) {
			brakes->AddBrakeTire (leftTire[i]);
			brakes->AddBrakeTire (rightTire[i]);
		}
		m_controller->SetBrakes(brakes);

		// add an engine
		// make the differential
		CustomVehicleControllerComponentEngine::dSingleAxelDifferential* axels[2];
		for (int i = 0; i < 2; i ++) {
			axels[i] = new CustomVehicleControllerComponentEngine::dSingleAxelDifferential (m_controller, leftTire[i], rightTire[i]);
		}
		CustomVehicleControllerComponentEngine::dMultiAxelDifferential* const differencial = new CustomVehicleControllerComponentEngine::dMultiAxelDifferential (m_controller, 2, axels);

		// make the gear Box
		dFloat fowardSpeedGearsBoxRatios[] = {parameters.GEAR_1, parameters.GEAR_1, parameters.GEAR_3};
		CustomVehicleControllerComponentEngine::dGearBox* const gearBox = new CustomVehicleControllerComponentEngine::dGearBox(m_controller, parameters.REVERSE_GEAR, sizeof (fowardSpeedGearsBoxRatios) / sizeof (fowardSpeedGearsBoxRatios[0]), fowardSpeedGearsBoxRatios); 
		CustomVehicleControllerComponentEngine* const engine = new CustomVehicleControllerComponentEngine (m_controller, gearBox, differencial);

		// the the default transmission type
		engine->SetTransmissionMode(m_automaticTransmission.GetPushButtonState());

		m_controller->SetEngine(engine);


		dFloat viperIdleRPM = parameters.IDLE_TORQUE_RPM;
		dFloat viperIdleTorquePoundPerFoot = parameters.IDLE_TORQUE;

		dFloat viperPeakTorqueRPM = parameters.PEAK_TORQUE_RPM;
		dFloat viperPeakTorquePoundPerFoot = parameters.PEAK_TORQUE;

		dFloat viperPeakHorsePowerRPM = parameters.PEAK_HP_RPM;
		dFloat viperPeakHorsePower = parameters.PEAK_HP;

		dFloat viperRedLineRPM = parameters.REDLINE_TORQUE_RPM;
		dFloat viperRedLineTorquePoundPerFoot = parameters.REDLINE_TORQUE;

		dFloat vehicleTopSpeedKPH = parameters.TIRE_TOP_SPEED_KMH;
		engine->InitEngineTorqueCurve (vehicleTopSpeedKPH, viperIdleTorquePoundPerFoot, viperIdleRPM, viperPeakTorquePoundPerFoot, viperPeakTorqueRPM, viperPeakHorsePower, viperPeakHorsePowerRPM, viperRedLineTorquePoundPerFoot, viperRedLineRPM);

		// do not forget to call finalize after all components are added or after any change is made to the vehicle
		m_controller->Finalize();
	}

	void SetTracks (CustomVehicleControllerBodyStateTire* const leftTire, CustomVehicleControllerBodyStateTire* const rightTire)
	{
		m_leftTrack.Init(this, "leftTrackPath", leftTire);
		m_rightTrack.Init(this, "rightTrackPath", rightTire);
	}

	void BuildTrackedVehicle (const VehicleParameters& parameters)
	{
		// Muscle cars have the front engine, we need to shift the center of mass to the front to represent that
		m_controller->SetCenterOfGravity (dVector (0.0f, parameters.COM_Y_OFFSET, 0.0f, 0.0f)); 

		// add all the tank tires
		dVector offset (0.0f, 0.15f, 0.0f, 0.0f);
		CustomVehicleControllerBodyStateTire* leftTire[8]; 
		CustomVehicleControllerBodyStateTire* rightTire[8]; 
		for (int i = 0; i < 8; i ++) {
			char name[32];
			dFloat width;
			dFloat radius;
			sprintf (name, "l_tire%d", i);
			CalculateTireDimensions (name, width, radius);
			// add the tire thread thickness
			radius += 0.1f;

			leftTire[i] = AddTire (name, offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
			sprintf (name, "r_tire%d", i);
			rightTire[i] = AddTire (name, offset, width, radius, parameters.TIRE_MASS, parameters.SUSPENSION_LENGTH, parameters.SUSPENSION_SPRING, parameters.SUSPENSION_DAMPER, parameters.LATERAL_STIFFNESS, parameters.LONGITUDINAL_STIFFNESS, parameters.ALIGNING_MOMENT_TRAIL, parameters.m_tireAligment);
		}

		m_controller->LinksTiresKinematically (8, leftTire);
		m_controller->LinksTiresKinematically (8, rightTire);

		// add vehicle brakes
		CustomVehicleControllerComponentBrake* const brakes = new CustomVehicleControllerComponentBrake (m_controller, parameters.BRAKE_TORQUE);
		brakes->AddBrakeTire (leftTire[0]);
		brakes->AddBrakeTire (rightTire[0]);
		m_controller->SetBrakes(brakes);

		// add a tank skid steering engine
		CustomVehicleControllerComponentTrackSkidSteering* const steering = new CustomVehicleControllerComponentTrackSkidSteering (m_controller, 1.0f, 10.0f * parameters.IDLE_TORQUE);
		steering->AddSteeringTire (leftTire[0]);
		steering->AddSteeringTire (rightTire[0]);
		m_controller->SetSteering(steering);

		// add an engine
		// make the differential
		CustomVehicleControllerComponentEngine::dTracksSkidDifferential* const differencial = new CustomVehicleControllerComponentEngine::dTracksSkidDifferential (m_controller, leftTire[0], rightTire[0]);

		// make the gear Box
		dFloat fowardSpeedGearsBoxRatios[] = {parameters.GEAR_1, parameters.GEAR_1, parameters.GEAR_3};
		CustomVehicleControllerComponentEngine::dGearBox* const gearBox = new CustomVehicleControllerComponentEngine::dGearBox(m_controller, parameters.REVERSE_GEAR, sizeof (fowardSpeedGearsBoxRatios) / sizeof (fowardSpeedGearsBoxRatios[0]), fowardSpeedGearsBoxRatios); 
		CustomVehicleControllerComponentEngine* const engine = new CustomVehicleControllerComponentEngine (m_controller, gearBox, differencial);

		// the the default transmission type
		engine->SetTransmissionMode(m_automaticTransmission.GetPushButtonState());

		m_controller->SetEngine(engine);


		dFloat viperIdleRPM = parameters.IDLE_TORQUE_RPM;
		dFloat viperIdleTorquePoundPerFoot = parameters.IDLE_TORQUE;

		dFloat viperPeakTorqueRPM = parameters.PEAK_TORQUE_RPM;
		dFloat viperPeakTorquePoundPerFoot = parameters.PEAK_TORQUE;

		dFloat viperPeakHorsePowerRPM = parameters.PEAK_HP_RPM;
		dFloat viperPeakHorsePower = parameters.PEAK_HP;

		dFloat viperRedLineRPM = parameters.REDLINE_TORQUE_RPM;
		dFloat viperRedLineTorquePoundPerFoot = parameters.REDLINE_TORQUE;

		dFloat vehicleTopSpeedKPH = parameters.TIRE_TOP_SPEED_KMH;
		engine->InitEngineTorqueCurve (vehicleTopSpeedKPH, viperIdleTorquePoundPerFoot, viperIdleRPM, viperPeakTorquePoundPerFoot, viperPeakTorqueRPM, viperPeakHorsePower, viperPeakHorsePowerRPM, viperRedLineTorquePoundPerFoot, viperRedLineRPM);

		// set up the tank Track
		SetTracks (leftTire[0], rightTire[0]);

		// do not forget to call finalize after all components are added or after any change is made to the vehicle
		m_controller->Finalize();
	}



	void ApplyPlayerControl ()
	{
		NewtonBody* const body = m_controller->GetBody();
		NewtonWorld* const world = NewtonBodyGetWorld(body);
		DemoEntityManager* const scene = (DemoEntityManager*) NewtonWorldGetUserData(world);
		NewtonDemos* const mainWindow = scene->GetRootWindow();

		CustomVehicleControllerComponentEngine* const engine = m_controller->GetEngine();
		CustomVehicleControllerComponentSteering* const steering = m_controller->GetSteering();
		CustomVehicleControllerComponentBrake* const brakes = m_controller->GetBrakes();
		CustomVehicleControllerComponentBrake* const handBrakes = m_controller->GetHandBrakes();

		// get the throttler input
		dFloat joyPosX;
		dFloat joyPosY;
		int joyButtons;

		int gear = engine ? engine->GetGear() : CustomVehicleControllerComponentEngine::dGearBox::m_newtralGear;
		dFloat steeringVal = 0.0f;
		dFloat engineGasPedal = 0.0f;
		dFloat brakePedal = 0.0f;
		dFloat handBrakePedal = 0.0f;
	
		bool hasJopytick = mainWindow->GetJoytickPosition (joyPosX, joyPosY, joyButtons);
		if (hasJopytick) {
			dAssert (0);
/*
			// apply a cubic attenuation to the joystick inputs
			joyPosX = joyPosX * joyPosX * joyPosX;
			joyPosY = joyPosY * joyPosY * joyPosY;

			steeringVal = joyPosX;
			brakePedal = (joyPosY < 0.0f) ? -joyPosY: 0.0f;
			engineGasPedal = (joyPosY >= 0.0f) ? joyPosY: 0.0f;

			gear += int (m_gearUpKey.UpdateTriggerJoystick(mainWindow, joyButtons & 2)) - int (m_gearDownKey.UpdateTriggerJoystick(mainWindow, joyButtons & 4));
			handBrakePedal = (joyButtons & 1) ? 1.0f : 0.0f;
*/			
		} else {
	
			// get keyboard controls
			if (mainWindow->GetKeyState ('W')) {
				engineGasPedal = 1.0f;
			}

			// see if HandBreale is on
			if (mainWindow->GetKeyState ('S')) {
				brakePedal = 1.0f;
			}

			// see if HandBreale is on
			if (mainWindow->GetKeyState (' ')) {
				handBrakePedal = 1.0f;
			}


			// get the steering input
			steeringVal = (dFloat (mainWindow->GetKeyState ('A')) - dFloat (mainWindow->GetKeyState ('D')));

/*
			// check for gear change (note key code for '>' = '.' and key code for '<' == ',')
			gear += int (m_gearUpKey.UpdateTriggerButton(mainWindow, '.')) - int (m_gearDownKey.UpdateTriggerButton(mainWindow, ','));
			// do driving heuristic for automatic transmission
			if (engine->GetTransmissionMode()) {
				dFloat speed = engine->GetSpeed();
				// check if vehicle is parked
				if ((dAbs (speed) < 1.0f) && !engineGasPedal && !brakePedal && !handBrakePedal) {
					handBrakePedal = 0.5f;
				}
			}
*/
		}

			
		// set the help key
		m_helpKey.UpdatePushButton (mainWindow, 'H');

		int reverseGear = m_reverseGear.UpdateTriggerButton (mainWindow, 'R') ? 1 : 0;

		// count the engine key switch
		m_engineKeySwitchCounter += m_engineKeySwitch.UpdateTriggerButton(mainWindow, 'Y');

		// check transmission type
		int toggleTransmission = m_automaticTransmission.UpdateTriggerButton (mainWindow, 0x0d) ? 1 : 0;

#if 0
	#if 1
		static FILE* file = fopen ("log.bin", "wb");                                         
		if (file) {
			fwrite (&m_engineKeySwitchCounter, sizeof (int), 1, file);
			fwrite (&toggleTransmission, sizeof (int), 1, file);
			fwrite (&reverseGear, sizeof (int), 1, file);
			fwrite (&gear, sizeof (int), 1, file);
			fwrite (&steeringVal, sizeof (dFloat), 1, file);
			fwrite (&engineGasPedal, sizeof (dFloat), 1, file);
			fwrite (&handBrakePedal, sizeof (dFloat), 1, file);
			fwrite (&brakePedal, sizeof (dFloat), 1, file);
			fflush(file);
		}
	#else 
		static FILE* file = fopen ("log.bin", "rb");
		if (file) {		
			fread (&m_engineKeySwitchCounter, sizeof (int), 1, file);
			fread (&toggleTransmission, sizeof (int), 1, file);
			fread (&reverseGear, sizeof (int), 1, file);
			fread (&gear, sizeof (int), 1, file);
			fread (&steeringVal, sizeof (dFloat), 1, file);
			fread (&engineGasPedal, sizeof (dFloat), 1, file);
			fread (&handBrakePedal, sizeof (dFloat), 1, file);
			fread (&brakePedal, sizeof (dFloat), 1, file);

		}
	#endif
#endif


		if (engine) {
			m_engineOldKeyState = engine->GetKey();
			engine->SetKey ((m_engineKeySwitchCounter & 1) ? true : false);

			if (toggleTransmission) {
				engine->SetTransmissionMode (!engine->GetTransmissionMode());
			}
			if (!engine->GetTransmissionMode()) {
				engine->SetGear(gear);
			}
			if (reverseGear) {
				if (engine->GetGear() == CustomVehicleControllerComponentEngine::dGearBox::m_reverseGear) {
					engine->SetGear(CustomVehicleControllerComponentEngine::dGearBox::m_newtralGear);
				} else {
					engine->SetGear(CustomVehicleControllerComponentEngine::dGearBox::m_reverseGear);
				}
			}
			engine->SetParam(engineGasPedal);
		}

		if (steering) {
			steering->SetParam(steeringVal);
		}
		if (brakes) {
			brakes->SetParam(brakePedal);
		}
		if (handBrakes) {
			handBrakes->SetParam(handBrakePedal);
		}
		
	}

	void ApplyNPCControl ()
	{
		// simple park NPC vehicle 
		CustomVehicleControllerComponentBrake* const brakes = m_controller->GetBrakes();
		if (brakes) {
			brakes->SetParam (0.5f);
		}
	}
	
	void Debug () const 
	{
		NewtonBody* const body = m_controller->GetBody();
		const CustomVehicleControllerBodyStateChassis& chassis = m_controller->GetChassisState ();

		dFloat scale = -4.0f / (chassis.GetMass() * DEMO_GRAVITY);
		dVector p0 (chassis.GetCenterOfMass());

		glDisable (GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glLineWidth(3.0f);
		glBegin(GL_LINES);


		// draw vehicle weight at the center of mass
		dFloat lenght = scale * chassis.GetMass() * DEMO_GRAVITY;
		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f (p0.m_x, p0.m_y, p0.m_z);
		glVertex3f (p0.m_x, p0.m_y - lenght, p0.m_z);

		// draw vehicle front dir
		glColor3f(1.0f, 1.0f, 1.0f);
		dVector r0 (p0 + chassis.GetMatrix()[1].Scale (0.5f));
		dVector r1 (r0 + chassis.GetMatrix()[0].Scale (1.0f));
		glVertex3f (r0.m_x, r0.m_y, r0.m_z);
		glVertex3f (r1.m_x, r1.m_y, r1.m_z);


		// draw the velocity vector, a little higher so that is not hidden by the vehicle mesh 
		dVector veloc;
		NewtonBodyGetVelocity(body, &veloc[0]);
		dVector q0 (p0 + chassis.GetMatrix()[1].Scale (1.0f));
		dVector q1 (q0 + veloc.Scale (0.25f));
		glColor3f(1.0f, 1.0f, 0.0f);
		glVertex3f (q0.m_x, q0.m_y, q0.m_z);
		glVertex3f (q1.m_x, q1.m_y, q1.m_z);
		
		for (CustomVehicleControllerBodyStateTire* node = m_controller->GetFirstTire(); node; node = m_controller->GetNextTire(node)) {
			const CustomVehicleControllerBodyStateTire& tire = *node;
			dVector p0 (tire.GetCenterOfMass());

			// offset the origin so that it is visible 
			const dMatrix& tireMatrix = tire.GetLocalMatrix ();
			p0 += chassis.GetMatrix()[2].Scale ((tireMatrix.m_posit.m_z > 0.0f ? 1.0f : -1.0f) * 0.25f);

			// draw the tire load 
			dVector p1 (p0 + tire.GetTireLoad().Scale (scale));
			glColor3f (0.0f, 0.0f, 1.0f);
			glVertex3f (p0.m_x, p0.m_y, p0.m_z);
			glVertex3f (p1.m_x, p1.m_y, p1.m_z);

			// show tire lateral force
			dVector p2 (p0 - tire.GetLateralForce().Scale (scale));
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f (p0.m_x, p0.m_y, p0.m_z);
			glVertex3f (p2.m_x, p2.m_y, p2.m_z);

			// show tire longitudinal force
			dVector p3 (p0 - tire.GetLongitudinalForce().Scale (scale));
			glColor3f (0.0f, 1.0f, 0.0f);
			glVertex3f (p0.m_x, p0.m_y, p0.m_z);
			glVertex3f (p3.m_x, p3.m_y, p3.m_z);
		}

		glEnd();

		glLineWidth(1.0f);

	}

	TrackSystem m_leftTrack;
	TrackSystem m_rightTrack;
	CustomVehicleController* m_controller;
	DemoEntityManager::ButtonKey m_helpKey;
	DemoEntityManager::ButtonKey m_gearUpKey;
	DemoEntityManager::ButtonKey m_gearDownKey;
	DemoEntityManager::ButtonKey m_reverseGear;
	DemoEntityManager::ButtonKey m_engineKeySwitch;
	DemoEntityManager::ButtonKey m_automaticTransmission;
	int m_gearMap[10];
	int m_engineKeySwitchCounter;
	bool m_engineOldKeyState;
	bool m_engineRPMOn;
};

class HeavyVehicleControllerManager: public CustomVehicleControllerManager
{
	public:
	HeavyVehicleControllerManager (NewtonWorld* const world)
		:CustomVehicleControllerManager (world)
		,m_externalView(true)
		,m_changeVehicle(false)
		,m_player (NULL) 
	{
		// hook a callback for 2d help display
		DemoEntityManager* const scene = (DemoEntityManager*) NewtonWorldGetUserData(world);
		scene->Set2DDisplayRenderFunction (RenderVehicleHud, this);
	}

	~HeavyVehicleControllerManager ()
	{
	}

	static void RenderVehicleHud (DemoEntityManager* const scene, void* const context, int lineNumber)
	{
		HeavyVehicleControllerManager* const me = (HeavyVehicleControllerManager*) context;
		me->RenderVehicleHud (scene, lineNumber);
	}


	void TestSpline () const
	{
		glDisable (GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		dFloat64 knots[] = {0.0f, 1.0f / 3.0f, 2.0f / 3.0f, 1.0f};
//dFloat64 knots[] = {0.0f, 1.0f};
		dBigVector control[] = 
		{
			dBigVector (200.0f, 200.0f, 0.0f, 1.0f),
			dBigVector (150.0f, 250.0f, 0.0f, 1.0f),
			dBigVector (250.0f, 300.0f, 0.0f, 1.0f),
			dBigVector (350.0f, 250.0f, 0.0f, 1.0f),
			dBigVector (250.0f, 150.0f, 0.0f, 1.0f),
			dBigVector (200.0f, 200.0f, 0.0f, 1.0f),
		};
		static dBezierSpline spline;
		if (!spline.GetControlPointArray()) {
			spline.CreateFromKnotVectorAndControlPoints(3, sizeof (knots) / sizeof (knots[0]), knots, control);
			dFloat64 u = (knots[1] + knots[2]) * 0.5f;
			spline.InsertKnot (u);
			spline.InsertKnot (u);
			spline.InsertKnot (u);
			spline.InsertKnot (u);
			spline.InsertKnot (u);
			spline.RemoveKnot (u, 1.0e-3f);
			//spline.RemoveKnot (u, 1.0e-3f);
		}

		const int segments = 20;
		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);
		dBigVector p0 (spline.CurvePoint(0.0f)) ;
		for (int i = 1; i <= segments; i ++) {
			dBigVector p1 (spline.CurvePoint(float (i) / segments)) ;
			glVertex3f (p0.m_x, p0.m_y, p0.m_z);
			glVertex3f (p1.m_x, p1.m_y, p1.m_z);
			p0 = p1;
		}
		glEnd();

		glColor3f(0.0f, 1.0f, 1.0f);
		glPointSize(4.0f);
		glBegin(GL_POINTS);
		for (int i = 0; i < spline.GetControlPointCount(); i ++) {
			dBigVector p (spline.GetControlPoint(i));
			glVertex3f (p.m_x, p.m_y, p.m_z);
		}
		glEnd();

		// recreate the spline from sample points at equally spaced distance 
		int pointCount = 4;
		dBigVector points[5];
		points[0] = spline.CurvePoint(0.0f / 3.0f) + dBigVector (300.0f, 0.0f, 0.0f, 0.0f);
		points[1] = spline.CurvePoint(1.0f / 3.0f) + dBigVector (300.0f, 0.0f, 0.0f, 0.0f);
		points[2] = spline.CurvePoint(2.0f / 3.0f) + dBigVector (300.0f, 0.0f, 0.0f, 0.0f);
		points[3] = spline.CurvePoint(3.0f / 3.0f) + dBigVector (300.0f, 0.0f, 0.0f, 0.0f);

		dBigVector derivP0 (spline.CurveDerivative(0.0f));
		dBigVector derivP1 (spline.CurveDerivative(1.0f));

		static dBezierSpline spline1;
		spline1.GlobalCubicInterpolation (pointCount, points, derivP0, derivP1);

		glColor3f(1.0f, 1.0f, 1.0f);
		glBegin(GL_LINES);
		p0 = spline1.CurvePoint(0.0f);
		for (int i = 1; i <= segments; i ++) {
			dFloat64 u = float (i) / segments;
			dBigVector p1 (spline1.CurvePoint(u));
			glVertex3f (p0.m_x, p0.m_y, p0.m_z);
			glVertex3f (p1.m_x, p1.m_y, p1.m_z);
			p0 = p1;
		}
		glEnd();


		glPointSize(4.0f);
		glBegin(GL_POINTS);
		glColor3f(0.0f, 1.0f, 1.0f);
		for (int i = 0; i < spline.GetControlPointCount(); i ++) {
			dBigVector p (spline1.GetControlPoint(i));
			glVertex3f (p.m_x, p.m_y, p.m_z);
		}

		glColor3f(1.0f, 0.0f, 0.0f);
		for (int i = 0; i < pointCount; i ++) {
			glVertex3f (points[i].m_x, points[i].m_y, points[i].m_z);
		}
		glEnd();
	}

	void DrawHelp(DemoEntityManager* const scene, int lineNumber) const
	{
		if (m_player->m_helpKey.GetPushButtonState()) {
			dVector color(1.0f, 1.0f, 0.0f, 0.0f);
			lineNumber = scene->Print (color, 10, lineNumber + 20, "Vehicle driving keyboard control:   Joystick control");
			lineNumber = scene->Print (color, 10, lineNumber + 20, "key switch          : 'Y'           start engine");
			lineNumber = scene->Print (color, 10, lineNumber + 20, "accelerator         : 'W'           stick forward");
			lineNumber = scene->Print (color, 10, lineNumber + 20, "brakes              : 'S'           stick back");
			lineNumber = scene->Print (color, 10, lineNumber + 20, "turn right          : 'D'           stick right");
			lineNumber = scene->Print (color, 10, lineNumber + 20, "turn right          : 'S'           stick left");
			lineNumber = scene->Print (color, 10, lineNumber + 20, "toggle Reverse Gear : 'R'           toggle reverse Gear");
			lineNumber = scene->Print (color, 10, lineNumber + 20, "toggle Vehicle      : 'V'           change vehicle");
			lineNumber = scene->Print (color, 10, lineNumber + 20, "hide help           : 'H'");
		}
	}


	void RenderVehicleHud (DemoEntityManager* const scene, int lineNumber) const
	{
		if (m_player) {
			// set to transparent color
			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			//print controllers help 
			DrawHelp(scene, lineNumber);

			// restore color and blend mode
			glDisable (GL_BLEND);
		}

//		TestSpline();
	}

	void SetAsPlayer (HeavyVehicleEntity* const player)
	{
		m_player = player;
	}


	virtual void PreUpdate (dFloat timestep)
	{
		// apply the vehicle controls, and all simulation time effect
		NewtonWorld* const world = GetWorld(); 
		DemoEntityManager* const scene = (DemoEntityManager*) NewtonWorldGetUserData(world);
		NewtonDemos* const mainWindow = scene->GetRootWindow();

		int changeVehicle = m_changeVehicle.UpdateTriggerButton (mainWindow, 'V') ? 1 : 0;
		if (changeVehicle) 
		{
			dListNode* nextPlater = GetLast();
			for (dListNode* ptr = GetFirst(); ptr; ptr = ptr->GetNext()) {
				CustomVehicleController* const controller = &ptr->GetInfo();
				NewtonBody* const body = controller->GetBody();
				HeavyVehicleEntity* const vehicleEntity = (HeavyVehicleEntity*) NewtonBodyGetUserData(body);
				if (vehicleEntity == m_player) {
					CustomVehicleController* const controller1 = &nextPlater->GetInfo();
					NewtonBody* const body1 = controller1->GetBody();
					SetAsPlayer((HeavyVehicleEntity*) NewtonBodyGetUserData(body1));
					break;
				}
				nextPlater = ptr;
			}
		}


		for (dListNode* ptr = GetFirst(); ptr; ptr = ptr->GetNext()) {
			CustomVehicleController* const controller = &ptr->GetInfo();

			NewtonBody* const body = controller->GetBody();
			HeavyVehicleEntity* const vehicleEntity = (HeavyVehicleEntity*) NewtonBodyGetUserData(body);

			if (vehicleEntity == m_player) {
				// do player control
				vehicleEntity->ApplyPlayerControl ();
			} else {
				// do no player control
				vehicleEntity->ApplyNPCControl ();
			}
		}

		// do the base class post update
		CustomVehicleControllerManager::PreUpdate(timestep);
	}

	virtual void PostUpdate (dFloat timestep)
	{
		// do the base class post update
		CustomVehicleControllerManager::PostUpdate(timestep);

		// update the visual transformation matrices for all vehicle tires
		for (dListNode* node = GetFirst(); node; node = node->GetNext()) {
			CustomVehicleController* const controller = &node->GetInfo();
			HeavyVehicleEntity* const vehicleEntity = (HeavyVehicleEntity*)NewtonBodyGetUserData (controller->GetBody());
			vehicleEntity->UpdateTireTransforms(timestep);
		}
		
		UpdateCamera (timestep);
	}


	void UpdateCamera (dFloat timestep)
	{
		if (m_player) {
			DemoEntityManager* const scene = (DemoEntityManager*) NewtonWorldGetUserData(GetWorld());
			DemoCamera* const camera = scene->GetCamera();
			dMatrix camMatrix (camera->GetNextMatrix ());
			dMatrix playerMatrix (m_player->GetNextMatrix());

			dVector frontDir (camMatrix[0]);
			dVector camOrigin; 
			if (m_externalView) {
				camOrigin = playerMatrix.m_posit + dVector(0.0f, HEAVY_VEHICLE_THIRD_PERSON_VIEW_HIGHT, 0.0f, 0.0f);
				camOrigin -= frontDir.Scale (HEAVY_VEHICLE_THIRD_PERSON_VIEW_DIST);
			} else {
				dAssert (0);
				//            camMatrix = camMatrix * playerMatrix;
				//            camOrigin = playerMatrix.TransformVector(dVector(-0.8f, ARTICULATED_VEHICLE_CAMERA_EYEPOINT, 0.0f, 0.0f));
			}

			camera->SetNextMatrix (*scene, camMatrix, camOrigin);
		}
	}

	// use this to display debug information about vehicle 
	void Debug () const
	{
		for (dListNode* ptr = GetFirst(); ptr; ptr = ptr->GetNext()) {
			CustomVehicleController* const controller = &ptr->GetInfo();
			HeavyVehicleEntity* const vehicleEntity = (HeavyVehicleEntity*)NewtonBodyGetUserData (controller->GetBody());
			vehicleEntity->Debug();
		}
	}

	bool m_externalView;
	DemoEntityManager::ButtonKey m_changeVehicle;
	HeavyVehicleEntity* m_player;
};

#endif
void MilitaryTransport (DemoEntityManager* const scene)
{
	dAssert(0);
/*

	// load the sky box
	scene->CreateSkyBox();

//	CreateLevelMesh (scene, "flatPlane.ngd", 1);
	CreateHeightFieldTerrain(scene, HEIGHTFIELD_DEFAULT_SIZE, HEIGHTFIELD_DEFAULT_CELLSIZE,
							  5.0f, 0.2f, 200.0f, -50.0f);

//	dMatrix camMatrix (dRollMatrix(-20.0f * 3.1416f /180.0f) * dYawMatrix(-45.0f * 3.1416f /180.0f));
	dMatrix location (dGetIdentityMatrix());
	location.m_posit = dVector (1000.0f, 100.0f, 1000.0f, 1.0f);
location.m_posit.m_x = 126.0f;
location.m_posit.m_y = 50.0f;
location.m_posit.m_z = 50.0f;

	location.m_posit = FindFloor (scene->GetNewton(), location.m_posit, 100.0f);
	location.m_posit.m_y += 2.0f;

	NewtonWorld* const world = scene->GetNewton();

	// create a vehicle controller manager
	HeavyVehicleControllerManager* const manager = new HeavyVehicleControllerManager (world);
	
	HeavyVehicleEntity* const heavyVehicle = new HeavyVehicleEntity (scene, manager, location, "lav-25.ngd", heavyTruck);
	heavyVehicle->BuildAllWheelDriveVehicle (heavyTruck);

	// add a second Vehicle
	location.m_posit.m_z += 4.0f;
	location.m_posit = FindFloor (scene->GetNewton(), location.m_posit, 100.0f);
	location.m_posit.m_y += 2.0f;
	HeavyVehicleEntity* const lightVehicle = new HeavyVehicleEntity (scene, manager, location, "buggy_.ngd", lightTruck);
	lightVehicle->BuildLightTruckVehicle (lightTruck);

	location.m_posit.m_z -= 12.0f;
	location.m_posit = FindFloor (scene->GetNewton(), location.m_posit, 100.0f);
	location.m_posit.m_y += 3.0f;
	HeavyVehicleEntity* const m1a1Tank = new HeavyVehicleEntity (scene, manager, location, "m1a1_.ngd", m1a1Param);
	m1a1Tank->BuildTrackedVehicle (m1a1Param);

	// set this vehicle as the player
	manager->SetAsPlayer(m1a1Tank);

//NewtonDestroyBody (m1a1Tank->m_controller->GetBody());
//manager->DestroyController(m1a1Tank->m_controller);
*/

/*
location.m_posit.m_z += 20.0f;
for (int i = 0; i < 5; i ++){
	location.m_posit.m_z += 12.0f;
	location.m_posit = FindFloor (scene->GetNewton(), location.m_posit, 100.0f);
	location.m_posit.m_y += 3.0f;
	HeavyVehicleEntity* const m1a1Tank1 = new HeavyVehicleEntity (scene, manager, location, "m1a1_.ngd", m1a1Param);
	m1a1Tank1->BuildTrackedVehicle (m1a1Param);
}
*/
	
//	dMatrix camMatrix (manager->m_player->GetNextMatrix());
//	scene->SetCameraMouseLock (true);
//	scene->SetCameraMatrix(camMatrix, camMatrix.m_posit);

	//	int defaultMaterialID = NewtonMaterialGetDefaultGroupID (scene->GetNewton());
	//	int count = 5;
	//	dMatrix shapeOffsetMatrix (dGetIdentityMatrix());
	//	dVector size (3.0f, 0.125f, 3.0f, 0.0f);
	//	AddPrimitiveArray(scene, 100.0f, location.m_posit, size, count, count, 5.0f, _BOX_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);

	//	size = dVector(1.0f, 0.5f, 1.0f, 0.0f);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _SPHERE_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _BOX_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _CAPSULE_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _CYLINDER_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _TAPERED_CAPSULE_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _TAPERED_CYLINDER_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _CHAMFER_CYLINDER_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _CONE_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _REGULAR_CONVEX_HULL_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	AddPrimitiveArray(scene, 10.0f, location.m_posit, size, count, count, 5.0f, _RANDOM_CONVEX_HULL_PRIMITIVE, defaultMaterialID, shapeOffsetMatrix);
	//	NewtonSerializeToFile (scene->GetNewton(), "C:/Users/Julio/Desktop/newton-dynamics/applications/media/xxxxx.bin");
}

