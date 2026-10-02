#include "transform.h"
#include "vector.h"
#include "quat.h"
#include "matrix.h"

FTransform::FTransform(const FQuat& Rotation, const FVector& Translation, const FVector& Scale3D) :Rotation(Rotation), Translation(Translation), Scale3D(Scale3D) {}
FTransform::FTransform() : Rotation(FQuat(0.f, 0.f, 0.f, 1.f)), Translation(FVector(0.0, 0.0, 0.0)), Scale3D(FVector(1.0f, 1.0f, 1.0f)) {}

bool FTransform::AnyHasNegativeScale(const FVector& InScale3D, const FVector& InOtherScale3D)
{
	return  (InScale3D.X < 0.f || InScale3D.Y < 0.f || InScale3D.Z < 0.f
		|| InOtherScale3D.X < 0.f || InOtherScale3D.Y < 0.f || InOtherScale3D.Z < 0.f);
}

FMatrix FTransform::ToMatrixWithScale() const
{
	FMatrix OutMatrix;
	OutMatrix.M[3][0] = Translation.X;
	OutMatrix.M[3][1] = Translation.Y;
	OutMatrix.M[3][2] = Translation.Z;

	const float x2 = Rotation.X + Rotation.X;
	const float y2 = Rotation.Y + Rotation.Y;
	const float z2 = Rotation.Z + Rotation.Z;
	{
		const float xx2 = Rotation.X * x2;
		const float yy2 = Rotation.Y * y2;
		const float zz2 = Rotation.Z * z2;

		OutMatrix.M[0][0] = (1.0f - (yy2 + zz2)) * Scale3D.X;
		OutMatrix.M[1][1] = (1.0f - (xx2 + zz2)) * Scale3D.Y;
		OutMatrix.M[2][2] = (1.0f - (xx2 + yy2)) * Scale3D.Z;
	}
	{
		const float yz2 = Rotation.Y * z2;
		const float wx2 = Rotation.W * x2;

		OutMatrix.M[2][1] = (yz2 - wx2) * Scale3D.Z;
		OutMatrix.M[1][2] = (yz2 + wx2) * Scale3D.Y;
	}
	{
		const float xy2 = Rotation.X * y2;
		const float wz2 = Rotation.W * z2;

		OutMatrix.M[1][0] = (xy2 - wz2) * Scale3D.Y;
		OutMatrix.M[0][1] = (xy2 + wz2) * Scale3D.X;
	}
	{
		const float xz2 = Rotation.X * z2;
		const float wy2 = Rotation.W * y2;

		OutMatrix.M[2][0] = (xz2 + wy2) * Scale3D.Z;
		OutMatrix.M[0][2] = (xz2 - wy2) * Scale3D.X;
	}

	OutMatrix.M[0][3] = 0.0f;
	OutMatrix.M[1][3] = 0.0f;
	OutMatrix.M[2][3] = 0.0f;
	OutMatrix.M[3][3] = 1.0f;

	return OutMatrix;
}

void FTransform::MultiplyUsingMatrixWithScale(FTransform* OutTransform, const FTransform* A, const FTransform* B)
{
	ConstructTransformFromMatrixWithDesiredScale(A->ToMatrixWithScale(), B->ToMatrixWithScale(), A->Scale3D * B->Scale3D, *OutTransform);
}

void FTransform::ConstructTransformFromMatrixWithDesiredScale(const FMatrix& AMatrix, const FMatrix& BMatrix, const FVector& DesiredScale, FTransform& OutTransform)
{
	FMatrix M = AMatrix * BMatrix;
	M.RemoveScaling();

	FVector SignedScale = DesiredScale.GetSignVector();

	M.SetAxis0(SignedScale.X * M.GetScaledAxisX());
	M.SetAxis1(SignedScale.Y * M.GetScaledAxisY());
	M.SetAxis2(SignedScale.Z * M.GetScaledAxisZ());

	FQuat Rotation = FQuat(M);
	Rotation.Normalize();

	OutTransform.Scale3D = DesiredScale;
	OutTransform.Rotation = Rotation;

	OutTransform.Translation = M.GetOrigin();
}

void FTransform::Multiply(FTransform* OutTransform, const FTransform* A, const FTransform* B)
{
	if (AnyHasNegativeScale(A->Scale3D, B->Scale3D))
	{
		MultiplyUsingMatrixWithScale(OutTransform, A, B);
	}
	else
	{
		OutTransform->Rotation = B->Rotation * A->Rotation;
		OutTransform->Scale3D = A->Scale3D * B->Scale3D;
		OutTransform->Translation = B->Rotation * (B->Scale3D * A->Translation) + B->Translation;
	}

}

FTransform FTransform::operator*(const FTransform& A) {
	FTransform OutTransform;
	Multiply(&OutTransform, this, &A);
	return OutTransform;
}

FVector FTransform::GetSafeScaleReciprocal(const FVector& InScale, float Tolerance)
{
	FVector SafeReciprocalScale;
	if (fabs(InScale.X) <= Tolerance)
	{
		SafeReciprocalScale.X = 0.f;
	}
	else
	{
		SafeReciprocalScale.X = 1 / InScale.X;
	}

	if (fabs(InScale.Y) <= Tolerance)
	{
		SafeReciprocalScale.Y = 0.f;
	}
	else
	{
		SafeReciprocalScale.Y = 1 / InScale.Y;
	}

	if (fabs(InScale.Z) <= Tolerance)
	{
		SafeReciprocalScale.Z = 0.f;
	}
	else
	{
		SafeReciprocalScale.Z = 1 / InScale.Z;
	}

	return SafeReciprocalScale;
}

void FTransform::GetRelativeTransformUsingMatrixWithScale(FTransform* OutTransform, const FTransform* Base, const FTransform* Relative)
{
	FMatrix AM = Base->ToMatrixWithScale();
	FMatrix BM = Relative->ToMatrixWithScale();
	FVector SafeRecipScale3D = GetSafeScaleReciprocal(Relative->Scale3D, SMALL_NUMBER);
	FVector DesiredScale3D = Base->Scale3D * SafeRecipScale3D;
	ConstructTransformFromMatrixWithDesiredScale(AM, BM.Inverse(), DesiredScale3D, *OutTransform);
}

FTransform FTransform::GetRelativeTransform(const FTransform& Other) const
{
	FTransform Result;

	if (AnyHasNegativeScale(Scale3D, Other.Scale3D))
	{
		GetRelativeTransformUsingMatrixWithScale(&Result, this, &Other);
	}
	else
	{
		FVector SafeRecipScale3D = GetSafeScaleReciprocal(Other.Scale3D, SMALL_NUMBER);
		Result.Scale3D = Scale3D * SafeRecipScale3D;

		if (Other.Rotation.IsNormalized() == false)
		{
			return FTransform();
		}

		FQuat Inverse = Other.Rotation.Inverse();
		Result.Rotation = Inverse * Rotation;

		Result.Translation = (Inverse * (Translation - Other.Translation)) * (SafeRecipScale3D);
	}

	return Result;
}