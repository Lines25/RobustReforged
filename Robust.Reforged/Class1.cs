using System.Runtime.InteropServices;
using System.Numerics;

namespace Robust.Reforged;

[StructLayout(LayoutKind.Sequential)]
public struct PhysicsBodyData
{
    public float PosX, PosY;
    public float VelX, VelY;
    public float AngVel;
    public float Friction;
    public float AngDamping;
    public float InvMass;
    public float InvI;
    public float ForceX, ForceY;
    public float Torque;
}

public static class ReforgedNative
{
    const string LibPath = "libreforged";

	[DllImport(LibPath)] public static extern void reforged_tick_begin();
	[DllImport(LibPath)] public static extern void reforged_tick_end();
	[DllImport(LibPath)] public static extern IntPtr reforged_hello();
	[DllImport(LibPath)] public static extern double reforged_tick_last_ms();
	[DllImport(LibPath)] public static extern double reforged_tick_avg_ms();
	[DllImport(LibPath)] public static extern int reforged_tick_is_spike(double threshold);
	[DllImport(LibPath)] public static extern void reforged_section_begin(string name);
	[DllImport(LibPath)] public static extern void reforged_section_end(string name);
	[DllImport(LibPath)] public static extern void reforged_sections_report(byte[] buf, int size);
	[DllImport(LibPath)] public static extern void reforged_sections_reset();

	[DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static unsafe extern void StepPhysicsParallel(PhysicsBodyData* bodies, int count, float frameTime);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static unsafe extern void IntegratePositionsNative(Vector2* positions, float* angles, Vector2* vels, float* angVels, int count, float dt, float maxVel, float maxAngVel);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static unsafe extern void IntegrateAllParallel(Vector2* positions, float* angles, Vector2* vels, float* angVels, float* forces, float* torques, float* invMasses, float* invIs, float* frictions, float* angDamps, int count, float dt, float maxVel, float maxAngVel);

	public static void Load()
	{
		var ptr = reforged_hello();
        var msg = Marshal.PtrToStringAnsi(ptr);
        Log($"reforged_hello(): {msg}");
	}

	public static void PrintSectionsReport()
	{
	    var buf = new byte[1024*4*4*4*4];
	    reforged_sections_report(buf, buf.Length);
	    Log("Section report:");
	    Console.WriteLine(System.Text.Encoding.ASCII.GetString(buf).TrimEnd('\0'));
	    reforged_sections_reset();
	}

	public static void Log(string message)
	{
	    Console.WriteLine($"[Reforged/Managed] {message}");
	}
}
