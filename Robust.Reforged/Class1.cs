using System;
using System.Runtime.InteropServices;
using System.Numerics;

namespace Robust.Reforged;

// TODO: Make this use 32/64-bit structures (for CPU cache to not miss)
// 33 bytes
[StructLayout(LayoutKind.Sequential)]
public struct PhysicsBodyData
{
    public float ForceX, ForceY, Torque;
    public float InvMass, InvI;
    public float LinearDamping, AngularDamping;
    public float GravityScale; 
    public int IsDynamic;
}

public static class ReforgedNative
{
    private const string LibPath = "libreforged";
    public static bool IsNativeEnabled { get; private set; } = false;


    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static unsafe extern void IntegrateVelocitiesNative(Vector2* vels, float* angVels, PhysicsBodyData* data, int count, float dt, float gravX, float gravY);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static unsafe extern void IntegratePositionsNative(Vector2* positions, float* angles, Vector2* vels, float* angVels, int count, float dt, float maxVel, float maxAngVel);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static unsafe extern void WarmStartNative(void* constraints, int count, float* linearVelocities, float* angularVelocities, int bodyOffset);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl)]
    public static unsafe extern void SolveVelocityConstraintsNative(void* constraints, int count, float* linearVelocities, float* angularVelocities, int bodyOffset);

    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_hello")]
    private static extern IntPtr raw_reforged_hello();
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_tick_begin")]
    private static extern void raw_reforged_tick_begin();
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_tick_end")]
    private static extern void raw_reforged_tick_end();
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_tick_last_ms")]
    private static extern double raw_reforged_tick_last_ms();
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_tick_avg_ms")]
    private static extern double raw_reforged_tick_avg_ms();
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_tick_is_spike")]
    private static extern int raw_reforged_tick_is_spike(double threshold);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_section_begin")]
    private static extern void raw_reforged_section_begin(string name);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_section_end")]
    private static extern void raw_reforged_section_end(string name);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_sections_report")]
    private static extern void raw_reforged_sections_report(byte[] buf, int size);
    [DllImport(LibPath, CallingConvention = CallingConvention.Cdecl, EntryPoint = "reforged_sections_reset")]
    private static extern void raw_reforged_sections_reset();

    public static void reforged_tick_begin()
    {
        if (IsNativeEnabled)
            raw_reforged_tick_begin();
    }

    public static void reforged_tick_end()
    {
        if (IsNativeEnabled)
            raw_reforged_tick_end();
    }

    public static double reforged_tick_last_ms()
    {
        return IsNativeEnabled ? raw_reforged_tick_last_ms() : 0.0;
    }

    public static double reforged_tick_avg_ms()
    {
        return IsNativeEnabled ? raw_reforged_tick_avg_ms() : 0.0;
    }

    public static int reforged_tick_is_spike(double threshold)
    {
        return IsNativeEnabled ? raw_reforged_tick_is_spike(threshold) : 0;
    }

    public static void reforged_section_begin(string name)
    {
        if (IsNativeEnabled)
            raw_reforged_section_begin(name);
    }

    public static void reforged_section_end(string name)
    {
        if (IsNativeEnabled)
            raw_reforged_section_end(name);
    }

    public static void reforged_sections_report(byte[] buf, int size)
    {
        if (IsNativeEnabled)
            raw_reforged_sections_report(buf, size);
    }

    public static void reforged_sections_reset()
    {
        if (IsNativeEnabled)
            raw_reforged_sections_reset();
    }

    public static void Load()
    {
        try 
        {
            var ptr = raw_reforged_hello();
            var msg = Marshal.PtrToStringAnsi(ptr);
            IsNativeEnabled = true;
            Log($"Native library loaded: {msg}");
        } 
        catch (DllNotFoundException) 
        {
            IsNativeEnabled = false;
            Log("Native library NOT FOUND. Using C# fallback (Are we client ?)");
        }
        catch (Exception e)
        {
            IsNativeEnabled = false;
            Log($"Native library error: {e.Message}. Using C# fallback (Are we client ?)");
        }
    }

    public static void PrintSectionsReport()
    {
        if (!IsNativeEnabled)
            return;

        var buf = new byte[1024 * 64]; // 64KB
        reforged_sections_report(buf, buf.Length);
        Log("Section report:");
        Console.WriteLine(System.Text.Encoding.ASCII.GetString(buf).TrimEnd('\0'));
        reforged_sections_reset();
    }

    public static void Log(string message)
    {
        Console.WriteLine($"[Reforged] {message}");
    }
}
