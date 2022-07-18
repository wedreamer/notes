# 设计模式

## 创建型
### 单例模式

- 懒汉式单例模式
```c#
// 使用lock加锁new
namespace SingletonPattern
{
    /// <summary>
    /// 单例类：一个构造对象很耗时耗资源类型
    /// 懒汉式单例模式
    /// </summary>
    public class Singleton
    {
        /// <summary>
        /// 构造函数耗时耗资源
        /// </summary>
        private Singleton()
        {
            long lResult = 0;
            for (int i = 0; i < 10000000; i++)
            {
                lResult += i;
            }
            Thread.Sleep(2000);
            Console.WriteLine("{0}被构造一次", this.GetType().Name);
        }

        /// <summary>
        /// 3 全局唯一静态  重用这个变量
        /// volatile 促进线程安全 让线程按顺序操作
        /// </summary>
        private static volatile Singleton _Singleton = null;
        //因为单例会有线程安全问题，所以会加锁的操作        
        private static readonly object Singleton_Lock = new object();
       
        /// <summary>
        /// 2 公开的静态方法提供对象实例
        /// 双重if加锁会提高性能
        /// </summary>
        /// <returns></returns>
        public static Singleton GetInstance()
        {
            if (_Singleton == null)//是_Singleton已经被初始化之后，就不要进入锁等待了
            {
                lock (Singleton_Lock)
                //保证任意时刻只有一个线程进入lock范围
                //也限制了并发，尤其是_Singleton已经被初始化之后
                {
                    if (_Singleton == null)//保证只实例化一次
                    {
                        _Singleton = new Singleton();
                    }
                }
            }
            return _Singleton;
        }

        //既然是单例，大家用的是同一个对象，用的是同一个方法，
       //如果并发还有线程安全问题，所以要保证线程安全必须lock加锁
        public int iTotal = 0;
    }
}
```

- 饿汉式
```c#
// 静态构造函数
namespace SingletonPattern
{
    /// <summary>
    /// 单例类：一个构造对象很耗时耗资源类型    /// 
    /// 饿汉式
    /// </summary>
    public class SingletonSecond
    {
        private static SingletonSecond _SingletonSecond = null;

        /// <summary>
        /// 1 构造函数耗时耗资源
        /// </summary>
        private SingletonSecond()
        {
            long lResult = 0;
            for (int i = 0; i < 10000000; i++)
            {
                lResult += i;
            }
            Thread.Sleep(1000);
            Console.WriteLine("{0}被构造一次", this.GetType().Name);
        }

        /// <summary>
        /// 静态构造函数:由CLR保证，程序第一次使用这个类型前被调用，且只调用一次
        /// 检测，初始化
        /// 写日志功能的文件夹检测
        /// XML配置文件
        /// </summary>
        static SingletonSecond()
        {
            _SingletonSecond = new SingletonSecond();
            Console.WriteLine("SingletonSecond 被启动");
        }
       
        public static SingletonSecond CreateInstance()
        {
            return _SingletonSecond;
        }//饿汉式  只要使用类就会被构造
    }
}

```

```c#
// 静态变量初始化时走私有构造函数
namespace SingletonPattern
{
    /// <summary>
    /// 单例类：一个构造对象很耗时耗资源类型
    /// 饿汉式
    /// </summary>
    public class SingletonThird
    {
        /// <summary>
        /// 构造函数耗时耗资源
        /// </summary>
        private SingletonThird()
        {
            long lResult = 0;
            for (int i = 0; i < 10000000; i++)
            {
                lResult += i;
            }
            Thread.Sleep(1000);
            Console.WriteLine("{0}被构造一次", this.GetType().Name);
        }

        /// <summary>
        /// 静态字段：在第一次使用这个类之前，由CLR保证，初始化且只初始化一次
        /// 这个比构造函数还早
        /// </summary>
        private static SingletonThird _SingletonThird = new SingletonThird();
        public static SingletonThird GetInstance()
        {
            return _SingletonThird;
        }//饿汉式  只要使用类就会被构造
    }
}

```

### 抽象工厂模式	

```c#
/// <summary>
/// 一个工厂负责一些产品的创建
/// 产品簇
/// 单一职责就是创建完整的产品簇
/// 
/// 继承抽象类后，必须显式的override父类的抽象方法
/// </summary>
public abstract class FactoryAbstract
{
    public abstract IRace CreateRace();
    public abstract IArmy CreateArmy();
    public abstract IHero CreateHero();
    public abstract IResource CreateResource();

    //public abstract ILuck CreateLuck();
}

/// <summary>
/// 一个工厂负责一些产品的创建
/// </summary>
public class HumanFactory : FactoryAbstract
{
    public override IRace CreateRace()
    {
        return new Human();
    }

    public override IArmy CreateArmy()
    {
        return new HumanArmy();
    }
    public override IHero CreateHero()
    {
        return new HumanHero();
    }
    public override IResource CreateResource()
    {
        return new HumanResource();
    }
}
/// <summary>
/// 一个工厂负责一些产品的创建
/// </summary>
public class ORCFactory : FactoryAbstract
{
    public override IRace CreateRace()
    {
        return new ORC();
    }

    public override IArmy CreateArmy()
    {
        return new ORCArmy();
    }
    public override IHero CreateHero()
    {
        return new ORCHero();
    }
    public override IResource CreateResource()
    {
        return new ORCResource();
    }
}
/// <summary>
/// 一个工厂负责一些产品的创建
/// </summary>
public class UndeadFactory : FactoryAbstract
{
    public override IRace CreateRace()
    {
        return new Undead();
    }
    public override IArmy CreateArmy()
    {
        return new UndeadArmy();
    }
    public override IHero CreateHero()
    {
        return new UndeadHero();
    }
    public override IResource CreateResource()
    {
        return new UndeadResource();
    }
}

```

### 建造者模式	

```c#
//建造者抽象类，定义了建造者的能力
public abstract class Builder
{
    public abstract void Dadiji();//打地基
    public abstract void QiZhuan();//砌砖
    public abstract void FenShua();//粉刷
}

/// <summary>
/// 技术好的建造者
/// </summary>
public class GoodBuilder : Builder
{
    private StringBuilder house = new StringBuilder();
    public override void Dadiji()
    {
        house.Append("深地基-->");
        //这里一般是new一个部件，添加到实例中，如 house.Diji=new Diji("深地基")
        //为了演示方便 用sringBuilder表示一个复杂的房子，string表示房子的部件
    }

    public override void FenShua()
    {
        house.Append("粉刷光滑-->");
    }

    public override void QiZhuan()
    {
        house.Append("砌砖整齐-->");
    }
    public string GetHouse()
    {
        return house.Append("好质量房子建成了！").ToString();
    }
}

/// <summary>
/// 技术差的建造者
/// </summary>
public class BadBuilder : Builder
{
    private StringBuilder house = new StringBuilder();
    public override void Dadiji()
    {
        house.Append("挖浅地基-->");
    }

    public override void FenShua()
    {
        house.Append("粉刷粗糙-->");
    }

    public override void QiZhuan()
    {
        house.Append("砌砖错乱-->");
    }
    public string GetHouse()
    {
        return house.Append("坏质量房子建成了！").ToString();
    }
}

// 监工
//监工类，制定盖房子的步骤
public class Director
{
    private Builder builder;
    public Director(Builder builder)
    {
        this.builder = builder;
    }

    //制定盖房子的流程，
    public void Construct()
    {
        builder.Dadiji();//先打地基
        builder.QiZhuan();//再砌砖
        builder.FenShua();//最后粉刷
    }
}

// 调用
class Program
{
    static void Main(string[] args)
    {
        //监工1派遣技术好的建造者盖房子
        GoodBuilder goodBuilder = new GoodBuilder();
        Director director1 = new Director(goodBuilder);
        director1.Construct();
        string house1 = goodBuilder.GetHouse();
        Console.WriteLine(house1);

        //监工2派遣技术差的建造者盖房子
        GoodBuilder badBuilder = new GoodBuilder();
        Director director2 = new Director(goodBuilder);
        director2.Construct();
        string house2 = goodBuilder.GetHouse();
        Console.WriteLine(house2);
        Console.ReadKey();
    }
}
```

### 工厂方法模式	

```c#
namespace FactoryMethod.Factory
{
    public interface IFactory
    {
        IRace CreateRace();
    }

    public class UndeadFactory : IFactory
    {
        public IRace CreateRace()
        {
            return new Undead();
        }
    }
    public class ORCFactory : IFactory
    {
        public IRace CreateRace()
        {
            return new ORC();
        }
    }
    public class NEFactory : IFactory
    {
        public IRace CreateRace()
        {
            return new NE();
        }
    }

    public class HumanFactory : IFactory
    {
        public virtual IRace CreateRace()
        {
            return new Human();
        }
    }
    public class HumanFactoryAdvanced : HumanFactory
    {
        public override IRace CreateRace()
        {
            Console.WriteLine("123");
            return new Human();
        }
    }
}

{
    IFactory factory = new HumanFactory();//包一层
    IRace race = factory.CreateRace();
   
}
{
    IFactory factory = new UndeadFactory();
    IRace race = factory.CreateRace();
}

```

### 原型模式	

```c#
namespace SingletonPattern
{
    /// <summary>
    /// 原型模式：单例的基础上升级了一下，把对象从内存层面复制了一下，
    /// 然后返回是个新对象，但是又不是new出来的（不走构造函数）
    /// </summary>
    public class Prototype
    {
        /// <summary>
        /// 构造函数耗时耗资源
        /// </summary>
        private Prototype()
        {
            long lResult = 0;
            for (int i = 0; i < 10000000; i++)
            {
                lResult += i;
            }
            Thread.Sleep(2000);
            Console.WriteLine("{0}被构造一次", this.GetType().Name);
        }

        /// <summary>
        /// 3 全局唯一静态  重用这个变量
        /// </summary>
        private static volatile Prototype _Prototype = new Prototype();

        /// <summary>
        /// 2 公开的静态方法提供对象实例
        /// </summary>
        /// <returns></returns>
        public static Prototype CreateInstance()
        {
            Prototype prototype = (Prototype)_Prototype.MemberwiseClone();
            return prototype;
        }
    }
}

```

## 结构型

### 适配器模式	

```c#
/// <summary>
/// 数据访问接口
/// </summary>
public interface IHelper
{
    void Add<T>();
    void Delete<T>();
    void Update<T>();
    void Query<T>();
}

public class SqlserverHelper : IHelper
{
    public void Add<T>()
    {
        Console.WriteLine("This is {0} Add", this.GetType().Name);
    }
    public void Delete<T>()
    {
        Console.WriteLine("This is {0} Delete", this.GetType().Name);
    }
    public void Update<T>()
    {
        Console.WriteLine("This is {0} Update", this.GetType().Name);
    }
    public void Query<T>()
    {
        Console.WriteLine("This is {0} Query", this.GetType().Name);
    }
}

public class MysqlHelper : IHelper
{
    public void Add<T>()
    {
        Console.WriteLine("This is {0} Add", this.GetType().Name);
    }
    public void Delete<T>()
    {
        Console.WriteLine("This is {0} Delete", this.GetType().Name);
    }
    public void Update<T>()
    {
        Console.WriteLine("This is {0} Update", this.GetType().Name);
    }
    public void Query<T>()
    {
        Console.WriteLine("This is {0} Query", this.GetType().Name);
    }
}

Console.WriteLine("*****************************");
{
    IHelper helper = new SqlserverHelper();
    helper.Add<Program>();
    helper.Delete<Program>();
    helper.Update<Program>();
    helper.Query<Program>();
}
Console.WriteLine("*****************************");
{
    IHelper helper = new MysqlHelper();
    helper.Add<Program>();
    helper.Delete<Program>();
    helper.Update<Program>();
    helper.Query<Program>();
}

```

### 桥接模式	

```c#
public abstract class Shape
{
    //形状内部包含了另一个维度：color
    protected IColor color;
    public void SetColor(IColor color)
    {
        this.color = color;
    }
    //设置形状
    public abstract void Draw();
}

/// <summary>
/// 圆形
/// </summary>
public class Circle : Shape
{
    public override void Draw()
    {
        color.Paint("圆形");
    }
}
/// <summary>
/// 长方形
/// </summary>
public class Rectangle : Shape
{
    public override void Draw()
    {
        color.Paint("长方形");
    }
}
/// <summary>
/// 三角形
/// </summary>
public class Triangle : Shape
{
    public override void Draw()
    {
        color.Paint("三角形");
    }
}

/// <summary>
/// 颜色接口
/// </summary>
public interface IColor
{
    void Paint(string shape);
}
/// <summary>
/// 蓝色
/// </summary>
public class Blue : IColor
{
    public void Paint(string shape)
    {
        Console.WriteLine($"蓝色的{shape}");
    }
}
/// <summary>
/// 黄色
/// </summary>
public class Yellow : IColor
{
    public void Paint(string shape)
    {
        Console.WriteLine($"黄色的{shape}");
    }
}
/// <summary>
/// 红色
/// </summary>
public class Red : IColor
{
    public void Paint(string shape)
    {
        Console.WriteLine($"红色的{shape}");
    }
}

class Program
{
    static void Main(string[] args)
    {
        Shape circle = new Circle();
        IColor blue = new Blue();
        circle.SetColor(blue);//设置颜色
        circle.Draw();//画图

        Shape triangle = new Triangle();
        triangle.SetColor(blue);
        triangle.Draw();

        Console.ReadKey();
    }
}

```

### 装饰器模式	

```c#
public abstract class AbstractStudent
{
    public int Id { get; set; }
    public string Name { get; set; }

    public abstract void Study();
}

/// <summary>
/// 一个普通的公开课学员,学习公开课
/// </summary>
public class StudentFree : AbstractStudent
{
    public override void Study()
    {
        //Console.WriteLine("上课前要预习");

        Console.WriteLine("{0} is a free student studying .net Free", base.Name);
    }
}

/// <summary>
/// 一个普通的vip学员,学习vip课程
/// </summary>
public class StudentVip : AbstractStudent
{
    /// <summary>
    /// 付费  上课前要预习   
    /// 上课学习
    /// </summary>
    public override void Study()
    {
        Console.WriteLine("{0} is a vip student studying .net Vip", base.Name);
    }
}

/// <summary>
/// 继承+组合
/// 装饰器的基类
/// 也是一个学员，继承了抽象类
/// </summary>
public class BaseStudentDecorator : AbstractStudent
{
    private AbstractStudent _Student = null;//用了组合加override
    public BaseStudentDecorator(AbstractStudent student)
    {
        this._Student = student;
    }

    public override void Study()
    {
        this._Student.Study();
    }
}

/// <summary>
 /// 父类是BaseStudentDecorator，爷爷类AbstractStudent
 /// </summary>
 public class StudentVideoDecorator : BaseStudentDecorator
 {
     public StudentVideoDecorator(AbstractStudent student)
         : base(student)//表示父类的构造函数
     {
     }
     public override void Study()
     {
         base.Study();
         Console.WriteLine("视频代码回看");
     }
 }

 /// <summary>
 /// 父类是BaseStudentDecorator，爷爷类AbstractStudent
 /// </summary>
 public class StudentHomeworkDecorator : BaseStudentDecorator
 {
     public StudentHomeworkDecorator(AbstractStudent student)
         : base(student)//表示父类的构造函数
     {

     }

     public override void Study()
     {
         base.Study();

         Console.WriteLine("巩固练习");
        
     }
 }

/// <summary>
/// 父类是BaseStudentDecorator，爷爷类AbstractStudent
/// </summary>
public class StudentPayDecorator : BaseStudentDecorator
{
    public StudentPayDecorator(AbstractStudent student)
        : base(student)//表示父类的构造函数
    {

    }

    public override void Study()
    {
        Console.WriteLine("付费");
        base.Study();
    }
}

AbstractStudent student = new StudentVip()
{
    Id = 666,
    Name = "加菲猫"
};

var decorator = new BaseStudentDecorator(student);

var decorator = new BaseStudentDecorator(student);//里氏替换
student = new BaseStudentDecorator(student);//引用替换一下

AbstractStudent student = new StudentVip()
{
    Id = 666,
    Name = "加菲猫"
};
student.Study();
//BaseStudentDecorator decorator = new BaseStudentDecorator(student);
//AbstractStudent decorator = new BaseStudentDecorator(student);//里氏替换
student = new BaseStudentDecorator(student);//引用替换一下
student = new StudentHomeworkDecorator(student);
student = new StudentVideoDecorator(student);
student.Study();

```

### 组合模式	

```c#
/// <summary>
/// 抽象部件 定义了树枝和树叶的公共属性和接口
/// </summary>
public abstract class Component
{
    public string name;
    public Component(string name)
    {
        this.name = name;
    }
    //添加子节点
    public abstract void Add(Component c);
    //删除子节点
    public abstract void Remove(Component c);
    //展示方法，dept为节点深度
    public abstract void Display(int dept);
}

//具体员工，树形结构的Leaf
public class Employee : Component
{
    public Employee(string name):base(name)
    {
        this.name = name;
    }
    //Leaf不能添加/删除子节点所以空实现
    public override void Add(Component c)
    {
    }
    public override void Remove(Component c)
    {

    }
    public override void Display(int dept)
    {
        Console.WriteLine(new string('-', dept)+name);
    }
}

/// <summary>
/// 部门类，相当于树枝
/// </summary>
public class Depart : Component
{
    public Depart(string name) : base(name)
    {
        this.name = name;
    }
    //添加子节点
    public List<Component> children=new List<Component>();
    public override void Add(Component c)
    {
        children.Add(c);
    }
    //删除子节点
    public override void Remove(Component c)
    {
        children.Remove(c);
    }
    //展示自己和和内部的所有子节点，这里是组合模式的核心
    public override void Display(int dept)
    {
        Console.WriteLine(new string('-',dept)+name);
        foreach (var item in children)
        {
            //这里用到了递归的思想
            item.Display(dept + 4);
        }
    }
}

class Program
{
    static void Main(string[] args)
    {
        Component DepartA = new Depart("A总公司");
        Component DepartAX = new Depart("AX部门");
        Component DepartAY = new Depart("AY部门");
        Component DepartAX1 = new Depart("AX1子部门");
        Component DepartAX2 = new Depart("AX2子部门");
        Component Ae1 = new Employee("公司直属员工1");
        Component AXe1= new Employee("AX部门员工1");
        Component AX1e1= new Employee("AX1部门员工1");
        Component AX1e2= new Employee("AX1部门员工2");
        Component AYe1= new Employee("AY部门员工1");
        Component AYe2= new Employee("AY部门员工2");
        DepartA.Add(Ae1);
        DepartA.Add(DepartAX);
        DepartA.Add(DepartAY);
        DepartAX.Add(AXe1);
        DepartAX.Add(DepartAX1);
        DepartAX.Add(DepartAX2);
        DepartAX1.Add(AX1e1);
        DepartAX1.Add(AX1e2);
        DepartAY.Add(AYe1);
        DepartAY.Add(AYe2);
        //遍历总公司
        DepartA.Display(1);
        Console.ReadKey();
    }
}

```

### 外观模式	

```c#
/// <summary>
/// CPU子系统
/// </summary>
public class CPU
{
    public void CPUStart()
    {
        Console.WriteLine("CPU is start...");
    }
    public void CPUShutdown()
    {
        Console.WriteLine("CPU is shot down...");
    }
}

/// <summary>
/// 内存子系统
/// </summary>
public class Memory
{
    public void MemoryStart()
    {
        Console.WriteLine("Memory is start...");
    }
    public void MemoryShutdown()
    {
        Console.WriteLine("Memory is shot down...");
    }
}

/// <summary>
/// 硬盘子系统
/// </summary>
public class Disk
{
    public void DiskStart()
    {
        Console.WriteLine("Disk is start...");
    }
    public void DiskShutdown()
    {
        Console.WriteLine("Disk is shot down...");
    }
}

/// <summary>
/// 电脑  facede角色
/// </summary>
public class Computer
{
    private CPU cpu;
    private Memory memory;
    private Disk disk;
    public Computer()
    {
        this.cpu = new CPU();
        this.memory = new Memory();
        this.disk = new Disk();
    }

    public void Start()
    {
        cpu.CPUStart();
        memory.MemoryStart();
        disk.DiskStart();
        Console.WriteLine("computer start end!");
    }
    public void Shutdown()
    {
        cpu.CPUShutdown();
        memory.MemoryShutdown();
        disk.DiskShutdown();
        Console.WriteLine("computer shutdown end!");
    }
}


class Program
{
    static void Main(string[] args)
    {
        Computer computer = new Computer();
        //开机
        computer.Start();
        Console.WriteLine();
        //关机
        computer.Shutdown();
        Console.ReadKey();
    }
}

```

### 享元模式	

```c#
///抽象车类
public abstract class Car
{
    //开车
    public abstract void Use(Driver d);
}

/// <summary>
/// 具体的车类
/// </summary>
public class RealCar : Car
{
    //颜色
    public string Color { get; set; }
    public RealCar(string color)
    {
        this.Color = color;
    }
    //开车
    public override void Use(Driver d)
    {
        Console.WriteLine($"{d.Name}开{this.Color}的车");
    }
}

/// <summary>
/// 车库
/// </summary>
public class CarFactory
{
    private  Dictionary<string, Car> carPool=new Dictionary<string, Car>();
    //初始的时候，只有红色和绿色两辆汽车
    public CarFactory()
    {
        carPool.Add("红色", new RealCar("红色"));
        carPool.Add("绿色", new RealCar("蓝色"));
    }
    //获取汽车
    public  Car GetCar(string key)
    {
        //如果车库有就用车库里的车，车库没有就买一个（new一个）
        if (!carPool.ContainsKey(key))
        {
            carPool.Add(key, new RealCar(key));
        }
        return carPool[key];
    }
}

/// <summary>
/// 司机类
/// </summary>
public class Driver
{
    public string Name { get; set; }
    public Driver(string name)
    {
        this.Name = name;
    }
}

class Program
{
    static void Main(string[] args)
    {
        CarFactory carFactory = new CarFactory();

        //小头爸爸开蓝色的车
        Driver d1 = new Driver("小头爸爸");
        Car c1=carFactory.GetCar("蓝色");
        c1.Use(d1);

        //扁头妈妈开蓝色的车
        Driver d2 = new Driver("扁头妈妈");
        Car c2 = carFactory.GetCar("蓝色");
        c2.Use(d2);

        if (c1.Equals(c2))
        {
            Console.WriteLine("小头爸爸和扁头妈妈开的是同一辆车");
        }

        //车库没有白色的车，就new一辆白色的车
        Driver d3 = new Driver("大头儿子");
        Car c3 = carFactory.GetCar("白色");
        c3.Use(d3);
        Console.ReadKey();
    }
}

```

### 代理模式	

```c#
/// <summary>
/// 业务接口
/// </summary>
public interface ISubject
{
    /// <summary>
    /// get
    /// </summary>
    /// <returns></returns>
    bool GetSomething();

    /// <summary>
    /// do
    /// </summary>
    void DoSomething();
}

/// <summary>
/// 一个耗时耗资源的对象方法
/// </summary>
public class RealSubject : ISubject
{
    public RealSubject()
    {
        Thread.Sleep(2000);
        long lResult = 0;
        for (int i = 0; i < 100000000; i++)
        {
            lResult += i;
        }
        Console.WriteLine("RealSubject被构造。。。");
    }

    /// <summary>
    /// 火车站查询火车票
    /// </summary>
    public bool GetSomething()
    {
        Console.WriteLine("坐车去火车站看看余票信息。。。");
        Thread.Sleep(3000);
        Console.WriteLine("到火车站，看到是有票的");
        return true;
    }

    /// <summary>
    /// 火车站买票
    /// </summary>
    public void DoSomething()
    {
        Console.WriteLine("开始排队。。。");
        Thread.Sleep(2000);
        Console.WriteLine("终于买到票了。。。");
    }
}

public class ProxySubject : ISubject
{
    //组合一下
    private static ISubject _Subject = new RealSubject();
    public void DoSomething()
    {
        try
        {
            Console.WriteLine("prepare DoSomething...");
            _Subject.DoSomething();
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex.Message);
            throw ex;
        }
    }

    private static Dictionary<string, bool> ProxyDictionary = new Dictionary<string, bool>();
    public bool GetSomething()
        {
            try
            {
                Console.WriteLine("prepare GetSomething...");
                string key = "Proxy_GetSomething";
                bool bResult = false;
                if (!ProxyDictionary.ContainsKey(key))
                {
                    bResult = _Subject.GetSomething();
                    ProxyDictionary.Add(key, bResult);
                }
                else
                {
                    bResult = ProxyDictionary[key];
                }
                return bResult;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                throw ex;
            }
        }
}

{
     Console.WriteLine("***********Proxy**************");
     ISubject subject = new ProxySubject();
     subject.GetSomething();
     //subject.DoSomething();
}

```

## 行为型

### 模板方法模式	

```c#
/// <summary>
/// 银行客户端
/// </summary>
public abstract class AbstractClient
{
    public void Query(int id, string name, string password)
    {
        if (this.CheckUser(id, password))
        {
            double balance = this.QueryBalance(id);
            double interest = this.CalculateInterest(balance);
            this.Show(name, balance, interest);
        }
        else
        {
            Console.WriteLine("账户密码错误");
        }
    }

    public bool CheckUser(int id, string password)
    {
        return DateTime.Now < DateTime.Now.AddDays(1);
    }

    public double QueryBalance(int id)
    {
        return new Random().Next(10000, 1000000);
    }

    /// <summary>
    /// 活期  定期  利率不同
    /// </summary>
    /// <param name="balance"></param>
    /// <returns></returns>
    public abstract double CalculateInterest(double balance);

    public virtual void Show(string name, double balance, double interest)
    {
        Console.WriteLine("尊敬的{0}客户，你的账户余额为：{1}，利息为{2}",
            name, balance, interest);
    }       
}

/// <summary>
/// 银行客户端
/// </summary>
public class ClientVip : AbstractClient
{
    /// <summary>
    /// 活期  定期  利率不同
    /// </summary>
    /// <param name="balance"></param>
    /// <returns></returns>
    public override double CalculateInterest(double balance)
    {
        return balance * 0.005;
    }

    public override void Show(string name, double balance, double interest)
    {
        Console.WriteLine("尊贵的{0} vip客户，您的账户余额为：{1}，利息为{2}",
            name, balance, interest);
    }
}

/// <summary>
/// 银行客户端
/// </summary>
public class ClientRegular : AbstractClient
{
    /// <summary>
    /// 活期  定期  利率不同
    /// </summary>
    /// <param name="balance"></param>
    /// <returns></returns>
    public override double CalculateInterest(double balance)
    {
        return balance * 0.003;
    }
}

```

### 命令模式	

```c#
// 教官，负责调用命令对象执行请求
public class Invoke
{
    public Command _command;

    public Invoke(Command command)
    {
        this._command = command;
    }

    public void ExecuteCommand()
    {
        _command.Action();
    }
}

// 命令抽象类
public abstract class Command 
{
    // 命令应该知道接收者是谁，所以有Receiver这个成员变量
    protected Receiver _receiver;

    public Command(Receiver receiver)
    {
        this._receiver = receiver;
    }

    // 命令执行方法
    public abstract void Action();
}

// 
public class ConcreteCommand :Command
{
    public ConcreteCommand(Receiver receiver)
        : base(receiver)
    { 
    }

    public override void Action()
    {
        // 调用接收的方法，因为执行命令的是学生
        _receiver.Run1000Meters();
    }
}

// 命令接收者——学生
public class Receiver
{
    public void Run1000Meters()
    {
        Console.WriteLine("跑1000米");
    }
}

// 院领导
class Program
{
    static void Main(string[] args)
    {
        // 初始化Receiver、Invoke和Command
        Receiver r = new Receiver();
        Command c = new ConcreteCommand(r);
        Invoke i = new Invoke(c);
        
        // 院领导发出命令
        i.ExecuteCommand();
    }
}


// 院领导
class Program
{
    static void Main(string[] args)
    {
        // 行为的请求者和行为的实现者之间呈现一种紧耦合关系
        Receiver r = new Receiver();

        r.Run1000Meters();
    }
}

public class Receiver
{
    // 操作
    public void Run1000Meters()
    {
        Console.WriteLine("跑1000米");
    }
}

```

### 迭代器模式	

```c#
class Program
{
    static void Main(string[] args)
    {
        ColorList colors = new ColorList();
        //foreach遍历自定义的ColorList类型
        foreach (var item in colors)
        {
            Console.WriteLine(item);
        }
        Console.ReadKey();

        
    }
    
}

/// <summary>
/// 自定义颜色集合，实现IEnumerable接口
/// </summary>
public class ColorList : IEnumerable
{
    //实现GetEnumerator接口方法
    public IEnumerator GetEnumerator()
    {
        string[] colors= { "red", "green", "blue", "pink" };
        for (int i = 0; i < colors.Length; i++)
        {
            //yield return的作用是指定下一项的内容
            yield return colors[i];
        }

        //想反向遍历时可以这样写
        //for (int i = colors.Length-1; i >=0; i--)
        //{
        //    yield return colors[i];
        //}
    }
}

```

### 观察者模式	

```c#
/// <summary>
/// 只是为了把多个对象产生关系，方便保存和调用
/// 方法本身其实没用
/// </summary>
public interface IObserver
{
    void Action();
}

public class Chicken : IObserver
{
    public void Action()
    {
        this.Woo();
    }
    public void Woo()
    {
        Console.WriteLine("{0} Woo", this.GetType().Name);
    }
}
 public class Dog : IObserver
 {
     public void Action()
     {
         this.Wang();
     }
     public void Wang()
     {
         Console.WriteLine("{0} Wang", this.GetType().Name);
     }
 }
public class Baby : IObserver
{
    public void Action()
    {
        this.Cry();
    }

    public void Cry()
    {
        Console.WriteLine("{0} Cry", this.GetType().Name);
    }
}

public class Cat
{
    public void Miao()
    {
        Console.WriteLine("{0} Miao.....", this.GetType().Name);
        
        new Chicken().Woo();
        new Baby().Cry();           
        new Dog().Wang();          
    }
   
    private List<IObserver> _ObserverList = new List<IObserver>();
    public void AddObserver(IObserver observer)
    {
        this._ObserverList.Add(observer);
    }
    public void MiaoObserver()
        {
            Console.WriteLine("{0} MiaoObserver.....", this.GetType().Name);
            if (this._ObserverList != null && this._ObserverList.Count > 0)
            {
                foreach (var item in this._ObserverList)
                {
                    item.Action();
                }
            }
        }

    private event Action MiaoHandler;
    public void MiaoEvent()
        {
            Console.WriteLine("{0} MiaoEvent.....", this.GetType().Name);
            if (this.MiaoHandler != null)
            {
                foreach (Action item in this.MiaoHandler.GetInvocationList())
                {
                    item.Invoke();
                }
            }
        }
}


{
    Console.WriteLine("***************Common******************");
    Cat cat = new Cat();
    cat.Miao();
}
{
    Console.WriteLine("***************Observer******************");
    Cat cat = new Cat();                   
    cat.AddObserver(new Chicken());
    cat.AddObserver(new Baby());                  
    cat.AddObserver(new Dog());                   
    cat.MiaoObserver();
}
{
    Console.WriteLine("***************Observer******************");
    Cat cat = new Cat();
    cat.AddObserver(new Chicken());
    cat.AddObserver(new Baby());                    
    cat.AddObserver(new Dog());                   
    cat.MiaoObserver();
}

```

### 中介者模式	

```c#
//抽象玩家类
public abstract class AbstractCardPlayer
{
    public int MoneyCount { get; set; }
    public AbstractCardPlayer()
    {
        this.MoneyCount = 0;
    }
    public abstract void ChangeCount(int count, AbstractCardPlayer other);
}
//玩家A类
public class PlayerA : AbstractCardPlayer
{
    public override void ChangeCount(int count, AbstractCardPlayer other)
    {
        this.MoneyCount += count;
        other.MoneyCount -= count;
    }
}
//玩家B类
public class PlayerB : AbstractCardPlayer
{
    public override void ChangeCount(int count, AbstractCardPlayer other)
    {
        this.MoneyCount += count;
        other.MoneyCount -= count;
    }
}
class Program
{
    static void Main(string[] args)
    {
        AbstractCardPlayer a = new PlayerA() { MoneyCount = 20 };
        AbstractCardPlayer b = new PlayerB() { MoneyCount = 20 };
        //玩家a赢了玩家b 5元
        Console.WriteLine("a赢了b5元");
        a.ChangeCount(5, b);
        Console.WriteLine($"玩家a现在有{a.MoneyCount}元");
        Console.WriteLine($"玩家b现在有{b.MoneyCount}元");
        //玩家b赢了玩家a 10元
        Console.WriteLine("b赢了a10元");
        b.ChangeCount(10, a);
        Console.WriteLine($"玩家a现在有{a.MoneyCount}元");
        Console.WriteLine($"玩家b现在有{b.MoneyCount}元");
        Console.ReadKey();
    }
}

//抽象玩家类
public abstract class AbstractCardPlayer
{
    public int MoneyCount { get; set; }
    public AbstractCardPlayer()
    {
        this.MoneyCount = 0;
    }
    public abstract void ChangeCount(int count, AbstractMediator mediator);
}
//玩家A类
public class PlayerA : AbstractCardPlayer
{
    //通过中介者来算账，不用直接找输家了
    public override void ChangeCount(int count, AbstractMediator mediator)
    {
        mediator.AWin(count);
    }
}
//玩家B类
public class PlayerB : AbstractCardPlayer
{
    public override void ChangeCount(int count, AbstractMediator mediator)
    {
        mediator.BWin(count);
    }
}
//抽象中介者
public abstract class AbstractMediator
{
    //中介者必须知道所有同事
    public AbstractCardPlayer A;
    public AbstractCardPlayer B;
    public AbstractMediator(AbstractCardPlayer a,AbstractCardPlayer b)
    {
        A = a;
        B = b;
    }
    public abstract void AWin(int count);
    public abstract void BWin(int count);
}
//具体中介者
public class Mediator : AbstractMediator
{
    public Mediator(AbstractCardPlayer a,AbstractCardPlayer b):base(a,b){}
    public override void AWin(int count)
    {
        A.MoneyCount += count;
        B.MoneyCount -= count;
    }
    public override void BWin(int count)
    {
        A.MoneyCount -= count;
        B.MoneyCount += count;
    }
}
class Program
{
    static void Main(string[] args)
    {
        AbstractCardPlayer a = new PlayerA() { MoneyCount = 20 };
        AbstractCardPlayer b = new PlayerB() { MoneyCount = 20 };
        AbstractMediator mediator = new Mediator(a, b);
        //玩家a赢了玩家b 5元
        Console.WriteLine("a赢了b5元");
        a.ChangeCount(5, mediator);
        Console.WriteLine($"玩家a现在有{a.MoneyCount}元");
        Console.WriteLine($"玩家b现在有{b.MoneyCount}元");
        //玩家b赢了玩家a 10元
        Console.WriteLine("b赢了a10元");
        b.ChangeCount(10, mediator);
        Console.WriteLine($"玩家a现在有{a.MoneyCount}元");
        Console.WriteLine($"玩家b现在有{b.MoneyCount}元");
        Console.ReadKey();
    }
}

```

### 备忘录模式	

```c#
// 联系人
public class ContactPerson
{
    public string Name { get; set; }
    public string MobileNum { get; set; }
}

// 发起人
public class MobileOwner
{
    // 发起人需要保存的内部状态
    public List<ContactPerson> ContactPersons { get; set; }

    public MobileOwner(List<ContactPerson> persons)
    {
        ContactPersons = persons;
    }

    // 创建备忘录，将当期要保存的联系人列表导入到备忘录中 
    public ContactMemento CreateMemento()
    {
        // 这里也应该传递深拷贝，new List方式传递的是浅拷贝，
        // 因为ContactPerson类中都是string类型,所以这里new list方式对ContactPerson对象执行了深拷贝
        // 如果ContactPerson包括非string的引用类型就会有问题，所以这里也应该用序列化传递深拷贝
        return new ContactMemento(new List<ContactPerson>(this.ContactPersons));
    }

    // 将备忘录中的数据备份导入到联系人列表中
    public void RestoreMemento(ContactMemento memento)
    {
        // 下面这种方式是错误的，因为这样传递的是引用，
        // 则删除一次可以恢复，但恢复之后再删除的话就恢复不了.
        // 所以应该传递contactPersonBack的深拷贝，深拷贝可以使用序列化来完成
        this.ContactPersons = memento.contactPersonBack;
    }

    public void Show()
    {
        Console.WriteLine("联系人列表中有{0}个人，他们是:", ContactPersons.Count);
        foreach (ContactPerson p in ContactPersons)
        {
            Console.WriteLine("姓名: {0} 号码为: {1}", p.Name, p.MobileNum);
        }
    }
}

// 备忘录
public class ContactMemento
{
    // 保存发起人的内部状态
    public List<ContactPerson> contactPersonBack;

    public ContactMemento(List<ContactPerson> persons)
    {
        contactPersonBack = persons;
    }
}

// 管理角色
public class Caretaker
{
    public ContactMemento ContactM { get; set; }
}

class Program
{
    static void Main(string[] args)
    {
        List<ContactPerson> persons = new List<ContactPerson>()
        {
            new ContactPerson() { Name= "Learning Hard", MobileNum = "123445"},
            new ContactPerson() { Name = "Tony", MobileNum = "234565"},
            new ContactPerson() { Name = "Jock", MobileNum = "231455"}
        };
        MobileOwner mobileOwner = new MobileOwner(persons);
        mobileOwner.Show();

        // 创建备忘录并保存备忘录对象
        Caretaker caretaker = new Caretaker();
        caretaker.ContactM = mobileOwner.CreateMemento();

        // 更改发起人联系人列表
        Console.WriteLine("----移除最后一个联系人--------");
        mobileOwner.ContactPersons.RemoveAt(2);
        mobileOwner.Show();

        // 恢复到原始状态
        Console.WriteLine("-------恢复联系人列表------");
        mobileOwner.RestoreMemento(caretaker.ContactM);
        mobileOwner.Show();

        Console.Read();
    }
}


namespace MultipleMementoPattern
{
    // 联系人
    public class ContactPerson
    {
        public string Name { get; set; }
        public string MobileNum { get; set; }
    }

    // 发起人
    public class MobileOwner
    {
        public List<ContactPerson> ContactPersons { get; set; }
        public MobileOwner(List<ContactPerson> persons)
        {
            ContactPersons = persons;
        }

        // 创建备忘录，将当期要保存的联系人列表导入到备忘录中 
        public ContactMemento CreateMemento()
        {
             // 这里也应该传递深拷贝，new List方式传递的是浅拷贝，
            // 因为ContactPerson类中都是string类型,所以这里new list方式对ContactPerson对象执行了深拷贝
            // 如果ContactPerson包括非string的引用类型就会有问题，所以这里也应该用序列化传递深拷贝
            return new ContactMemento(new List<ContactPerson>(this.ContactPersons));
        }

        // 将备忘录中的数据备份导入到联系人列表中
        public void RestoreMemento(ContactMemento memento)
        {
            if (memento != null)
            {
                // 下面这种方式是错误的，因为这样传递的是引用，
                // 则删除一次可以恢复，但恢复之后再删除的话就恢复不了.
                // 所以应该传递contactPersonBack的深拷贝，深拷贝可以使用序列化来完成
                this.ContactPersons = memento.ContactPersonBack;
            }    
        }
        public void Show()
        {
            Console.WriteLine("联系人列表中有{0}个人，他们是:", ContactPersons.Count);
            foreach (ContactPerson p in ContactPersons)
            {
                Console.WriteLine("姓名: {0} 号码为: {1}", p.Name, p.MobileNum);
            }
        }
    }

    // 备忘录
    public class ContactMemento
    {
        public List<ContactPerson> ContactPersonBack {get;set;}
        public ContactMemento(List<ContactPerson> persons)
        {
            ContactPersonBack = persons;
        }
    }

    // 管理角色
    public class Caretaker
    {
        // 使用多个备忘录来存储多个备份点
        public Dictionary<string, ContactMemento> ContactMementoDic { get; set; }
        public Caretaker()
        {
            ContactMementoDic = new Dictionary<string, ContactMemento>();
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            List<ContactPerson> persons = new List<ContactPerson>()
            {
                new ContactPerson() { Name= "Learning Hard", MobileNum = "123445"},
                new ContactPerson() { Name = "Tony", MobileNum = "234565"},
                new ContactPerson() { Name = "Jock", MobileNum = "231455"}
            };

            MobileOwner mobileOwner = new MobileOwner(persons);
            mobileOwner.Show();

            // 创建备忘录并保存备忘录对象
            Caretaker caretaker = new Caretaker();
            caretaker.ContactMementoDic.Add(DateTime.Now.ToString(), mobileOwner.CreateMemento());

            // 更改发起人联系人列表
            Console.WriteLine("----移除最后一个联系人--------");
            mobileOwner.ContactPersons.RemoveAt(2);
            mobileOwner.Show();

            // 创建第二个备份
            Thread.Sleep(1000);
            caretaker.ContactMementoDic.Add(DateTime.Now.ToString(), mobileOwner.CreateMemento());

            // 恢复到原始状态
            Console.WriteLine("-------恢复联系人列表,请从以下列表选择恢复的日期------");
            var keyCollection = caretaker.ContactMementoDic.Keys;
            foreach (string k in keyCollection)
            {
                Console.WriteLine("Key = {0}", k);
            }
            while (true)
            {
                Console.Write("请输入数字,按窗口的关闭键退出:");
                
                int index = -1;
                try
                {
                    index = Int32.Parse(Console.ReadLine());
                }
                catch
                {
                    Console.WriteLine("输入的格式错误");
                    continue;
                }
                
                ContactMemento contactMentor = null;
                if (index < keyCollection.Count && caretaker.ContactMementoDic.TryGetValue(keyCollection.ElementAt(index), out contactMentor))
                {
                    mobileOwner.RestoreMemento(contactMentor);
                    mobileOwner.Show();
                }
                else
                {
                    Console.WriteLine("输入的索引大于集合长度！");
                }
            }     
        }
    }
}

```

### 解释器模式	

```c#
namespace InterpreterPattern
{
    // 抽象表达式
    public abstract class Expression
    {
        protected Dictionary<string, int> table = new Dictionary<string, int>(9);

        protected Expression()
        {
            table.Add("一", 1);
            table.Add("二", 2);
            table.Add("三", 3);
            table.Add("四", 4);
            table.Add("五", 5);
            table.Add("六", 6);
            table.Add("七", 7);
            table.Add("八", 8);
            table.Add("九", 9);
        }

        public virtual void Interpreter(Context context)
        {
            if (context.Statement.Length == 0)
            {
                return;
            }

            foreach (string key in table.Keys)
            {
                int value = table[key];

                if (context.Statement.EndsWith(key + GetPostFix()))
                {
                    context.Data += value * this.Multiplier();
                    context.Statement = context.Statement.Substring(0, context.Statement.Length - this.GetLength());
                }
                if (context.Statement.EndsWith("零"))
                {
                    context.Statement = context.Statement.Substring(0, context.Statement.Length - 1);
                }
            }
        }

        public abstract string GetPostFix();

        public abstract int Multiplier();

        //这个可以通用，但是对于个位数字例外，所以用虚方法
        public virtual int GetLength()
        {
            return this.GetPostFix().Length + 1;
        }
    }

    //个位表达式
    public sealed class GeExpression : Expression
    {
        public override string GetPostFix()
        {
            return "";
        }

        public override int Multiplier()
        {
            return 1;
        }

        public override int GetLength()
        {
            return 1;
        }
    }

    //十位表达式
    public sealed class ShiExpression : Expression
    {
        public override string GetPostFix()
        {
            return "十";
        }

        public override int Multiplier()
        {
            return 10;
        }
    }

    //百位表达式
    public sealed class BaiExpression : Expression
    {
        public override string GetPostFix()
        {
            return "百";
        }

        public override int Multiplier()
        {
            return 100;
        }
    }

    //千位表达式
    public sealed class QianExpression : Expression
    {
        public override string GetPostFix()
        {
            return "千";
        }

        public override int Multiplier()
        {
            return 1000;
        }
    }

    //万位表达式
    public sealed class WanExpression : Expression
    {
        public override string GetPostFix()
        {
            return "万";
        }

        public override int Multiplier()
        {
            return 10000;
        }

        public override void Interpreter(Context context)
        {
            if (context.Statement.Length == 0)
            {
                return;
            }

            ArrayList tree = new ArrayList();

            tree.Add(new GeExpression());
            tree.Add(new ShiExpression());
            tree.Add(new BaiExpression());
            tree.Add(new QianExpression());

            foreach (string key in table.Keys)
            {
                if (context.Statement.EndsWith(GetPostFix()))
                {
                    int temp = context.Data;
                    context.Data = 0;

                    context.Statement = context.Statement.Substring(0, context.Statement.Length - this.GetLength());

                    foreach (Expression exp in tree)
                    {
                        exp.Interpreter(context);
                    }
                    context.Data = temp + context.Data * this.Multiplier();
                }
            }
        }
    }

    //亿位表达式
    public sealed class YiExpression : Expression
    {
        public override string GetPostFix()
        {
            return "亿";
        }

        public override int Multiplier()
        {
            return 100000000;
        }

        public override void Interpreter(Context context)
        {
            ArrayList tree = new ArrayList();

            tree.Add(new GeExpression());
            tree.Add(new ShiExpression());
            tree.Add(new BaiExpression());
            tree.Add(new QianExpression());

            foreach (string key in table.Keys)
            {
                if (context.Statement.EndsWith(GetPostFix()))
                {
                    int temp = context.Data;
                    context.Data = 0;
                    context.Statement = context.Statement.Substring(0, context.Statement.Length - this.GetLength());

                    foreach (Expression exp in tree)
                    {
                        exp.Interpreter(context);
                    }
                    context.Data = temp + context.Data * this.Multiplier();
                }
            }
        }
    }

    //环境上下文
    public sealed class Context
    {
        private string _statement;
        private int _data;

        public Context(string statement)
        {
            this._statement = statement;
        }

        public string Statement
        {
            get { return this._statement; }
            set { this._statement = value; }
        }

        public int Data
        {
            get { return this._data; }
            set { this._data = value; }
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            string roman = "五亿七千三百零二万六千四百五十二";
            //分解：((五)亿)((七千)(三百)(零)(二)万)
            //((六千)(四百)(五十)(二))

            Context context = new Context(roman);
            ArrayList tree = new ArrayList();

            tree.Add(new GeExpression());
            tree.Add(new ShiExpression());
            tree.Add(new BaiExpression());
            tree.Add(new QianExpression());
            tree.Add(new WanExpression());
            tree.Add(new YiExpression());

            foreach (Expression exp in tree)
            {
                exp.Interpreter(context);
            }

            Console.Write(context.Data);

            Console.Read();
        }
    }
}

```

### 状态者模式	

```c#
namespace StatePatternSample
{
    public class Account
    {
        public State State {get;set;}
        public string Owner { get; set; }
        public Account(string owner)
        {
            this.Owner = owner;
            this.State = new SilverState(0.0, this);
        }

        public double Balance { get {return State.Balance; }} // 余额
        // 存钱
        public void Deposit(double amount)
        {
            State.Deposit(amount);
            Console.WriteLine("存款金额为 {0:C}——", amount);
            Console.WriteLine("账户余额为 =:{0:C}", this.Balance);
            Console.WriteLine("账户状态为: {0}", this.State.GetType().Name);
            Console.WriteLine();
        }

        // 取钱
        public void Withdraw(double amount)
        {
            State.Withdraw(amount);
             Console.WriteLine("取款金额为 {0:C}——",amount);
            Console.WriteLine("账户余额为 =:{0:C}", this.Balance);
            Console.WriteLine("账户状态为: {0}", this.State.GetType().Name);
            Console.WriteLine();
        }

        // 获得利息
        public void PayInterest()
        {
            State.PayInterest();
            Console.WriteLine("Interest Paid --- ");
            Console.WriteLine("账户余额为 =:{0:C}", this.Balance);
            Console.WriteLine("账户状态为: {0}", this.State.GetType().Name);
            Console.WriteLine();
        }
    }

    // 抽象状态类
    public abstract class State
    {
        // Properties
        public Account Account { get; set; }
        public double Balance { get; set; } // 余额
        public double Interest { get; set; } // 利率
        public double LowerLimit { get; set; } // 下限
        public double UpperLimit { get; set; } // 上限

        public abstract void Deposit(double amount); // 存款
        public abstract void Withdraw(double amount); // 取钱
        public abstract void PayInterest(); // 获得的利息
    }

    // Red State意味着Account透支了
    public class RedState : State
    {
        public RedState(State state)
        {
            // Initialize
            this.Balance = state.Balance;
            this.Account = state.Account;
            Interest = 0.00;
            LowerLimit = -100.00;
            UpperLimit = 0.00;
        }

        // 存款
        public override void Deposit(double amount)
        {
            Balance += amount;
            StateChangeCheck();
        }
        // 取钱
        public override void Withdraw(double amount)
        {
            Console.WriteLine("没有钱可以取了！");
        }

        public override void PayInterest()
        {
            // 没有利息
        }

        private void StateChangeCheck()
        {
            if (Balance > UpperLimit)
            {
                Account.State = new SilverState(this);
            }
        }
    }

    // Silver State意味着没有利息得
    public class SilverState :State
    {
        public SilverState(State state)
            : this(state.Balance, state.Account)
        { 
        }

        public SilverState(double balance, Account account)
        {
            this.Balance = balance;
            this.Account = account;
            Interest = 0.00;
            LowerLimit = 0.00;
            UpperLimit = 1000.00;
        }

        public override void Deposit(double amount)
        {
            Balance += amount;
            StateChangeCheck();
        }
        public override void Withdraw(double amount)
        {
            Balance -= amount;
            StateChangeCheck();
        }

        public override void PayInterest()
        {
            Balance += Interest * Balance;
            StateChangeCheck();
        }

        private void StateChangeCheck()
        {
            if (Balance < LowerLimit)
            {
                Account.State = new RedState(this);
            }
            else if (Balance > UpperLimit)
            {
                Account.State = new GoldState(this);
            }
        }     
    }

    // Gold State意味着有利息状态
    public class GoldState : State
    {
        public GoldState(State state)
        {
            this.Balance = state.Balance;
            this.Account = state.Account;
            Interest = 0.05;
            LowerLimit = 1000.00;
            UpperLimit = 1000000.00;
        }
        // 存钱
        public override void Deposit(double amount)
        {
            Balance += amount;
            StateChangeCheck();
        }
        // 取钱
        public override void Withdraw(double amount)
        {
            Balance -= amount;
            StateChangeCheck();
        }
        public override void PayInterest()
        {
            Balance += Interest * Balance;
            StateChangeCheck();
        }

        private void StateChangeCheck()
        {
            if (Balance < 0.0)
            {
                Account.State = new RedState(this);
            }
            else if (Balance < LowerLimit)
            {
                Account.State = new SilverState(this);
            }
        }
    }

    class App
    {
        static void Main(string[] args)
        {
            // 开一个新的账户
            Account account = new Account("Learning Hard");

            // 进行交易
            // 存钱
            account.Deposit(1000.0);
            account.Deposit(200.0);
            account.Deposit(600.0);

            // 付利息
            account.PayInterest();

            // 取钱
            account.Withdraw(2000.00);
            account.Withdraw(500.00);
            
            // 等待用户输入
            Console.ReadKey();
        }
    }
}

```

### 策略模式	

```c#
namespace StrategyPattern
{
    // 所得税计算策略
    public interface ITaxStragety
    {
        double CalculateTax(double income);
    }

    // 个人所得税
    public class PersonalTaxStrategy : ITaxStragety
    {
        public double CalculateTax(double income)
        {
            return income * 0.12;
        }
    }

    // 企业所得税
    public class EnterpriseTaxStrategy : ITaxStragety
    {
        public double CalculateTax(double income)
        {
            return (income - 3500) > 0 ? (income - 3500) * 0.045 : 0.0;
        }
    }

    public class InterestOperation
    {
        private ITaxStragety m_strategy;
        public InterestOperation(ITaxStragety strategy)
        {
            this.m_strategy = strategy;
        }

        public double GetTax(double income)
        {
            return m_strategy.CalculateTax(income);
        }
    }

    class App
    {
        static void Main(string[] args)
        {
            // 个人所得税方式
            InterestOperation operation = new InterestOperation(new PersonalTaxStrategy());
            Console.WriteLine("个人支付的税为：{0}", operation.GetTax(5000.00));

            // 企业所得税
            operation = new InterestOperation(new EnterpriseTaxStrategy());
            Console.WriteLine("企业支付的税为：{0}", operation.GetTax(50000.00));

            Console.Read();
        }
    }
}

```

### 责任链模式	

```c#
if (context.Hour <= 8)
{
    Console.WriteLine("PM审批通过");
}
else if (context.Hour <= 16)
{
    Console.WriteLine("部门主管审批通过");
}
else if (context.Hour <= 32)
{
    Console.WriteLine("公司主管审批通过");
}
else
{
    Console.WriteLine("************");
}

/// <summary>
/// 请假申请
/// </summary>
public class ApplyContext
{
    public int Id { get; set; }
    public string Name { get; set; }
    /// <summary>
    /// 请假时长
    /// </summary>
    public int Hour { get; set; }
    public string Description { get; set; }


    public bool AuditResult { get; set; }
    public string AuditRemark { get; set; }
}

public abstract class AbstractAuditor
{
    public string Name { get; set; }
    public abstract void Audit(ApplyContext context);

    private AbstractAuditor _NextAuditor = null;
    public void SetNext(AbstractAuditor auditor)
    {
        this._NextAuditor = auditor;
    }
    protected void AuditNext(ApplyContext context)
    {
        if (this._NextAuditor != null)
        {
            this._NextAuditor.Audit(context);
        }
        else
        {
            context.AuditResult = false;
            context.AuditRemark = "不允许请假！";
        }
    }
}

public class PM : AbstractAuditor
{
    public override void Audit(ApplyContext context)
    {
        Console.WriteLine($"This is {this.GetType().Name} {this.Name} Audit");
        if (context.Hour <= 8)
        {
            context.AuditResult = true;
            context.AuditRemark = "允许请假！";
        }
        else
        {
            base.AuditNext(context);
        }
    }
}

public class Charge: AbstractAuditor
{
    public override void Audit(ApplyContext context)
    {
        Console.WriteLine($"This is {this.GetType().Name} {this.Name} Audit");
        if (context.Hour <= 16)
        {
            context.AuditResult = true;
            context.AuditRemark = "允许请假！";
        }
        else
        {
            base.AuditNext(context);
        }
    }
}

public class Manager : AbstractAuditor
{
    public override void Audit(ApplyContext context)
    {
        Console.WriteLine($"This is {this.GetType().Name} {this.Name} Audit");
        if (context.Hour <= 24)
        {
            context.AuditResult = true;
            context.AuditRemark = "允许请假！";
        }
        else
        {
            base.AuditNext(context);
        }
    }
}

public class AuditorBuilder
{
    /// <summary>
    /// 那就反射+配置文件
    /// 链子的组成都可以通过配置文件
    /// </summary>
    /// <returns></returns>
    public static AbstractAuditor Build()
    {
        AbstractAuditor pm = new PM()
        {
            Name = "张琪琪"
        };
        AbstractAuditor charge = new Charge()
        {
            Name = "吴可可"
        };         
        AbstractAuditor ceo = new CEO()
        {
            Name = "加菲猫"
        };

        pm.SetNext(pm);
        charge.SetNext(charge);           
        ceo.SetNext(ceo);
        return pm;
    }
}

ApplyContext context = new ApplyContext()
{
    Id = 506,
    Name = "小新",
    Hour = 32,
    Description = "我周一要请假回家",
    AuditResult = false,
    AuditRemark = ""
};

AbstractAuditor auditor = AuditorBuilder.Build();
auditor.Audit(context);
if (!context.AuditResult)
{
    Console.WriteLine("不干了！");
}

```

### 访问者模式	

```c#
namespace DonotUsevistorPattern
{
    // 抽象元素角色
    public abstract class Element
    {      
        public abstract void Print();
    }

    // 具体元素A
    public class ElementA : Element
    {    
        public override void Print()
        {
            Console.WriteLine("我是元素A");
        }
    }

    // 具体元素B
    public class ElementB : Element
    {
        public override void Print()
        {
            Console.WriteLine("我是元素B");
        }
    }

    // 对象结构
    public class ObjectStructure
    {
        private ArrayList elements = new ArrayList();

        public ArrayList Elements
        {
            get { return elements; }
        }

        public ObjectStructure()
        {
            Random ran = new Random();
            for (int i = 0; i < 6; i++)
            {
                int ranNum = ran.Next(10);
                if (ranNum > 5)
                {
                    elements.Add(new ElementA());
                }
                else
                {
                    elements.Add(new ElementB());
                }
            }
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            ObjectStructure objectStructure = new ObjectStructure();
            // 遍历对象结构中的对象集合，访问每个元素的Print方法打印元素信息
            foreach (Element e in objectStructure.Elements)
            {
                e.Print();
            }

            Console.Read();
        }
    }
}

namespace VistorPattern
{
    // 抽象元素角色
    public abstract class Element
    {
        public abstract void Accept(IVistor vistor);
        public abstract void Print();
    }

    // 具体元素A
    public class ElementA :Element
    {
        public override void Accept(IVistor vistor)
        {
            // 调用访问者visit方法
            vistor.Visit(this);
        }
        public override void Print()
        {
            Console.WriteLine("我是元素A");
        }
    }
    
    // 具体元素B
    public class ElementB :Element
    {
        public override void Accept(IVistor vistor)
        {
            vistor.Visit(this);
        }
        public override void Print()
        {
            Console.WriteLine("我是元素B");
        }
    }

    // 抽象访问者
    public interface IVistor 
    {
        void Visit(ElementA a);
        void Visit(ElementB b);
    }

    // 具体访问者
    public class ConcreteVistor :IVistor
    {
        // visit方法而是再去调用元素的Accept方法
        public void Visit(ElementA a)
        {
            a.Print();
        }
        public void Visit(ElementB b)
        {
            b.Print();
        }
    }

    // 对象结构
    public class ObjectStructure
    {
        private ArrayList elements = new ArrayList();

        public ArrayList Elements
        {
            get { return elements; }
        }
     
        public ObjectStructure()
        {
            Random ran = new Random();
            for (int i = 0; i < 6; i++)
            {
                int ranNum = ran.Next(10);
                if (ranNum > 5)
                {
                    elements.Add(new ElementA());
                }
                else
                {
                    elements.Add(new ElementB());
                }
            }
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            ObjectStructure objectStructure = new ObjectStructure();
            foreach (Element e in objectStructure.Elements)
            {
                // 每个元素接受访问者访问
                e.Accept(new ConcreteVistor());
            }

            Console.Read();
        }
    }
}
```
