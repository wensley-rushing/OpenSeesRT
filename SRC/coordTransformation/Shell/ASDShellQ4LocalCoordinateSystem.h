/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
** ****************************************************************** */
//
// Original implementation: Massimo Petracca (ASDEA)
//
// Implementation of a local coordinate system for 4-node shells
//
#ifndef ASDShellQ4LocalCoordinateSystem_h
#define ASDShellQ4LocalCoordinateSystem_h

#include <Quaternion.h>
#include <Vector3D.h>
#include <array>
#include <vector>

/** ASDShellQ4LocalCoordinateSystem
*
* This class represent the local coordinate system of any element whose geometry
* is a 4-node Quadrilateral in 3D space
*/
class ASDShellQ4LocalCoordinateSystem
{

public:

	typedef Vector3D Vector3Type;

	typedef ASDQuaternion<double> QuaternionType;

	typedef std::vector<Vector3Type> Vector3ContainerType;

	typedef Matrix MatrixType;
	constexpr static int nen = 4;

public:

  template<class Vec3T>
	ASDShellQ4LocalCoordinateSystem(
		const Vec3T& P1global,
		const Vec3T& P2global,
		const Vec3T& P3global,
		const Vec3T& P4global,
		double alpha = 0.0)
		: m_P(4)
		, m_orientation(3, 3)
	{
		// compute the central point
		m_center  = P1global;
		m_center += P2global;
		m_center += P3global;
		m_center += P4global;
		m_center *= 0.25;

		// compute the Normal vector at the element center
		// as the cross product of the 2 diagonals.
		// While normalizing the normal vector save its norm to compute the area.
		// Note: the norm should be divided by 2 because it's computed from the
		// cross product of the diagonals, which gives twice the area!

		// compute the diagonal vectors
		Vector3D d13 = P3global - P1global;
		Vector3D d24 = P4global - P2global;

		Vector3D e3 = d13.cross(d24);
		m_area = e3.normalize();
		m_area /= 2.0;

		// compute the local X direction parallel to the projection of the side 1-2 onto
		// the local xy plane.
		Vector3D e1 = P2global - P1global;
		double e1_dot_e3 = e1.dot(e3);
		e1 -= e1_dot_e3 * e3;

		// if user defined local rotation is included...
		if (std::abs(alpha) > 0.0)
		    QuaternionType::FromAxisAngle(e3[0], e3[1], e3[2], alpha).rotateVector(e1);

		e1.normalize();

		// finally compute the local Y direction to be orthogonal to both X and Z local directions
		Vector3Type e2 = e3.cross(e1);
		e2.normalize();

		// form the 3x3 transformation matrix (the transposed actually...)
		for (int i = 0; i < 3; i++) {
          m_orientation(0, i) = e1[i];
          m_orientation(1, i) = e2[i];
          m_orientation(2, i) = e3[i];
		}

		// transform global coordinates to the local coordinate system
		for (int i = 0; i < 3; i++) {
			m_P[0][i] = m_orientation(i, 0) * (P1global[0] - m_center[0]) 
                + m_orientation(i, 1) * (P1global[1] - m_center[1]) 
                + m_orientation(i, 2) * (P1global[2] - m_center[2]);
			m_P[1][i] = m_orientation(i, 0) * (P2global[0] - m_center[0]) 
                + m_orientation(i, 1) * (P2global[1] - m_center[1])
                + m_orientation(i, 2) * (P2global[2] - m_center[2]);
			m_P[2][i] = m_orientation(i, 0) * (P3global[0] - m_center[0]) 
                + m_orientation(i, 1) * (P3global[1] - m_center[1])
                + m_orientation(i, 2) * (P3global[2] - m_center[2]);
			m_P[3][i] = m_orientation(i, 0) * (P4global[0] - m_center[0]) 
                + m_orientation(i, 1) * (P4global[1] - m_center[1]) 
                + m_orientation(i, 2) * (P4global[2] - m_center[2]);
		}
	}

public:

	inline const Vector3ContainerType& Nodes() const { return m_P; }

	inline const Vector3Type& P1()const { return m_P[0]; }
	inline const Vector3Type& P2()const { return m_P[1]; }
	inline const Vector3Type& P3()const { return m_P[2]; }
	inline const Vector3Type& P4()const { return m_P[3]; }
	inline const Vector3Type& Center()const { return m_center; }

	inline double X1()const { return m_P[0][0]; }
	inline double X2()const { return m_P[1][0]; }
	inline double X3()const { return m_P[2][0]; }
	inline double X4()const { return m_P[3][0]; }

	inline double Y1()const { return m_P[0][1]; }
	inline double Y2()const { return m_P[1][1]; }
	inline double Y3()const { return m_P[2][1]; }
	inline double Y4()const { return m_P[3][1]; }

	inline double Z1()const { return m_P[0][2]; }
	inline double Z2()const { return m_P[1][2]; }
	inline double Z3()const { return m_P[2][2]; }
	inline double Z4()const { return m_P[3][2]; }

	inline double X(size_t i) const { return m_P[i][0]; }
	inline double Y(size_t i) const { return m_P[i][1]; }
	inline double Z(size_t i) const { return m_P[i][2]; }

	inline double Area() const { return m_area; }

	inline const MatrixType& Orientation() const { return m_orientation; }

	inline Vector3Type Vx() const { return Vector3Type{{m_orientation(0, 0), m_orientation(0, 1), m_orientation(0, 2)}} ; }
	inline Vector3Type Vy() const { return Vector3Type{{m_orientation(1, 0), m_orientation(1, 1), m_orientation(1, 2)}} ; }
	inline Vector3Type Vz() const { return Vector3Type{{m_orientation(2, 0), m_orientation(2, 1), m_orientation(2, 2)}} ; }

	inline double WarpageFactor()const { return Z1(); }
	inline bool IsWarped()const { return std::abs(WarpageFactor()) > 0.0; }

	template <typename MatT>
	inline void 
	ComputeTotalRotationMatrix(MatT& R) const
	{
	  // Note: R is 24x24
	  // R.Zero();
      for (size_t k = 0; k < nen*2; k++) {
            size_t i = k * 3;
            R(i, i    ) = m_orientation(0, 0);   
			R(i, i + 1) = m_orientation(0, 1);   
			R(i, i + 2) = m_orientation(0, 2);
            R(i + 1, i    ) = m_orientation(1, 0);
			R(i + 1, i + 1) = m_orientation(1, 1);
			R(i + 1, i + 2) = m_orientation(1, 2);
            R(i + 2, i    ) = m_orientation(2, 0);
			R(i + 2, i + 1) = m_orientation(2, 1);
			R(i + 2, i + 2) = m_orientation(2, 2);
      }
	}

	inline void 
	ComputeTotalWarpageMatrix(MatrixType& W, double wf) const
	{
		constexpr size_t mat_size = 24;
		if (W.noRows() != mat_size || W.noCols() != mat_size)
			W.resize(mat_size, mat_size);

		W.Zero();
		for (size_t i = 0; i < mat_size; i++)
			W(i, i) = 1.0;

		W(0, 4) = -wf;
		W(1, 3) =  wf;

		W(6, 10) =  wf;
		W(7,  9) = -wf;

		W(12, 16) = -wf;
		W(13, 15) = wf;

		W(18, 22) =  wf;
		W(19, 21) = -wf;
	}

	inline void
	ComputeTotalWarpageMatrix(MatrixType& W) const {
		ComputeTotalWarpageMatrix(W, WarpageFactor());
	}

private:

	Vector3ContainerType m_P;
	Vector3Type m_center;
	MatrixType m_orientation;
	double m_area;

};

template<class TStream>
inline static TStream& operator << (TStream& s, const ASDShellQ4LocalCoordinateSystem& lc)
{
    s << "    " << "{\"points\": [";
    for (size_t i = 0; i < 4; i++) {
        s << "  [" << lc.X(i) << ", " << lc.Y(i) << "] ";
    }
    s << "],";
    s << " \"normal\": [";
    for (int i=0; i<3; i++) {
        s << lc.Vz()[i];
        if (i < 2)
        s << ", ";
    }
    s << "]}\n";
    return s;
}

#endif // !ASDShellQ4LocalCoordinateSystem_h
